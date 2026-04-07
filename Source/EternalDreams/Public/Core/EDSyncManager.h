#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EDSyncManager.generated.h"

UENUM(BlueprintType)
enum class EDataLoadPhase : uint8
{
	NotStarted      UMETA(DisplayName = "시작 전"),
	LoadingLobby    UMETA(DisplayName = "로비 데이터 로딩"),
	LobbyReady      UMETA(DisplayName = "로비 준비 완료"),
	
	LoadingItem     UMETA(DisplayName = "아이템 데이터 로딩"),
	LoadingMonster  UMETA(DisplayName = "몬스터 데이터 로딩"),
	LoadingAbility  UMETA(DisplayName = "어빌리티 데이터 로딩"),
	
	Completed       UMETA(DisplayName = "완료"),
	Failed          UMETA(DisplayName = "실패")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllDataLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataPhaseChanged, EDataLoadPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDataLoadProgress, int32, LoadedCount, int32, TotalCount);

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDSyncManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// ================================================================
    // UGameInstanceSubsystem 인터페이스
    // ================================================================
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
	
	/**
	 * UEDSyncManager의 전역 싱글톤 인스턴스
	 * 
	 * 
	 * @param WorldContextObject 현재 World의 컨텍스트 오브젝트 ex) this
	 * @return					 UEDSyncManager 인스턴스 포인터
	 * 
	 * 사용 예
	 * UEDSyncManager::Get(this)->InitializeGameData();
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|SyncManager", meta = (WorldContext = "WorldContextObject"))
    static UEDSyncManager* Get(const UObject* WorldContextObject);

	/**
	 * 게임 데이터 전체를 단계별로 비동기 로드하는 진입점 함수
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|SyncManager")
    void InitializeGameData();

    /**
     * 타입과 AssetName으로 캐시에서 즉시 반환
     * 반드시 IsDataReady() == true 상태에서 호출할 것
     *
     * 사용 예
     * UMonsterData* Goblin = SyncManager->GetData<UMonsterData>(TEXT("Wolf"));
     */
    template<typename T>
    T* GetData(const FName& AssetName) const;

    /**
     * 특정 타입의 모든 에셋을 배열로 반환
     * 사용 예
     * TArray<UItemData*> AllItems = SyncManager->GetAllDataOfType<UItemData>(ItemAssetType);
     */
    template<typename T>
    TArray<T*> GetAllDataOfType(const FPrimaryAssetType& AssetType) const;

    // 로딩 상태 조회
    UFUNCTION(BlueprintPure, Category = "ED|SyncManager")
    EDataLoadPhase GetCurrentPhase() const { return CurrentPhase; }
	
    UFUNCTION(BlueprintPure, Category = "ED|SyncManager")
    bool IsDataReady() const { return CurrentPhase == EDataLoadPhase::Completed; }

    /** 로딩 UI용 프로그레스바 */
    UFUNCTION(BlueprintPure, Category = "ED|SyncManager")
    float GetLoadProgress() const;

    // ================================================================
    // 델리게이트
    // ================================================================

    /** 
     * 모든 데이터 로드 완료 시 방송
     * 게임 시작 신호 
	 */
    UPROPERTY(BlueprintAssignable, Category = "ED|SyncManager")
    FOnAllDataLoaded OnAllDataLoaded;

    /** 
     * 단계가 바뀔 때마다 방송
     * 로딩 화면 텍스트 업데이트용 
     */
    UPROPERTY(BlueprintAssignable, Category = "ED|SyncManager")
    FOnDataPhaseChanged OnPhaseChanged;

    /** 
     * 에셋 하나 완료될 때마다 방송  
     * 프로그레스 바 업데이트용 
     */
    UPROPERTY(BlueprintAssignable, Category = "ED|SyncManager")
    FOnDataLoadProgress OnLoadProgress;

    // ================================================================
    // Asset Type 상수 (Project Settings > Asset Manager 등록 이름과 일치)
    // ================================================================
    static const FPrimaryAssetType CoreAssetType;
    static const FPrimaryAssetType ItemAssetType;
    static const FPrimaryAssetType MonsterAssetType;
    static const FPrimaryAssetType AbilityAssetType;
private:
	// ================================================================
	// 단계별 로드 함수
	// ================================================================
	void LoadPhase_Core();
	void LoadPhase_Item();
	void LoadPhase_Monster();
	void LoadPhase_Ability();

	// 각 단계 완료 콜백
	void OnCoreDataLoaded();
	void OnItemDataLoaded();
	void OnMonsterDataLoaded();
	void OnAbilityDataLoaded();

	// 공통 캐싱 헬퍼 -> 특정 타입의 로드된 에셋들을 DataCache에 저장
	void CacheLoadedAssets(const FPrimaryAssetType& AssetType);

	// 상태 전이 헬퍼
	void SetPhase(EDataLoadPhase NewPhase);

	/**
	 * 에셋 캐시
	 * Key: "AssetType:AssetName" 식으로 사용 (ex. "MonsterData:Goblin")
	 * Value: 로드된 UObject* 포인터
	 */
	UPROPERTY()
	TMap<FString, TObjectPtr<UObject>> DataCache;

	/**
	 * 핸들 컨테이너 -> 멤버변수로 관리 소멸 방지
	 * Key: 단계 이름
	 */
	TMap<FName, TSharedPtr<FStreamableHandle>> PhaseHandles;

	// ================================================================
	// 내부 상태
	// ================================================================
	EDataLoadPhase CurrentPhase = EDataLoadPhase::NotStarted;
	int32 TotalAssetsToLoad     = 0;
	int32 LoadedAssetsCount     = 0;
};

// ================================================================
// 템플릿 구현부
// ================================================================

template<typename T>
T* UEDSyncManager::GetData(const FName& AssetName) const
{
	// "ClassName:AssetName" 형식으로 키 조합
	const FString Key = FString::Printf(TEXT("%s:%s"),
		*T::StaticClass()->GetName(), *AssetName.ToString());

	if (const TObjectPtr<UObject>* Found = DataCache.Find(Key))
	{
		return Cast<T>(Found->Get());
	}

	UE_LOG(LogTemp, Warning,
		TEXT("[EDSyncManager] GetData - 캐시에서 찾을 수 없음: %s"), *Key);
	return nullptr;
}

template<typename T>
TArray<T*> UEDSyncManager::GetAllDataOfType(const FPrimaryAssetType& AssetType) const
{
	TArray<T*> Result;

	for (const auto& Pair : DataCache)
	{
		// Key 앞부분이 타입 이름과 일치하는 것만 필터
		if (Pair.Key.StartsWith(AssetType.ToString()))
		{
			if (T* Casted = Cast<T>(Pair.Value.Get()))
			{
				Result.Add(Casted);
			}
		}
	}
	return Result;
}