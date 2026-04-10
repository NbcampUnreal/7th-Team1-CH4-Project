#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/StreamableManager.h"
#include "EDGameDataSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDataLoadPhase : uint8
{
	NotStarted      UMETA(DisplayName = "시작 전"),
	LoadingLobby    UMETA(DisplayName = "로비 데이터 로딩"),
	LobbyReady      UMETA(DisplayName = "로비 준비 완료"),
	
	LoadingItem     UMETA(DisplayName = "아이템 데이터 로딩"),
	LoadingMonster  UMETA(DisplayName = "몬스터 데이터 로딩"),
	
	Completed       UMETA(DisplayName = "완료"),
	ReturningToLobby UMETA(DisplayName = "로비 복귀 중")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllDataLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllLobbyDataLoaded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDataPhaseChanged, EDataLoadPhase, NewPhase);

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDGameDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// ================================================================
	// 데이터 접근 API
	// ================================================================
	
	/**
	* FPrimaryAssetId로 캐시에서 즉시 반환
	* 사용 예
	* UMonsterData* Wolf = UEDGameDataSubsystem->GetData<UMonsterData>(
	*	FPrimaryAssetId(TEXT("MonsterData"), TEXT("Wolf"))
	* );
	*/
	template<typename T>
	T* GetData(const FPrimaryAssetId& Id) const;

	/**
	 * 특정 타입의 모든 에셋을 배열로 반환
	 * 사용 예
	 * TArray<UItemData*> AllItems = EDGameDataSubsystem->GetAllDataOfType<UItemData>(
	 *	FPrimaryAssetType(TEXT("ItemData"))
	 * );
	 */
	template<typename T>
	TArray<T*> GetAllDataOfType(const FPrimaryAssetType& AssetType) const;
	
public:
	// ================================================================
    // UGameInstanceSubsystem 인터페이스
    // ================================================================
	
	// GameInstanceSubsystem 수명주기 진입시점 자동 호출
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// GameInstanceSubsystem 수명주기 종료시점 자동 호출
    virtual void Deinitialize() override;
	
public:
	// ================================================================
	// 공개 API
	// ================================================================
	
	/**
	 * UEDGameDataSubsystem의 전역 싱글톤 인스턴스
	 * 사용 예
	 * UEDGameDataSubsystem::Get(this)->InitializeGameData();
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|EDGameDataSubsystem", meta = (WorldContext = "WorldContextObject"))
    static UEDGameDataSubsystem* Get(const UObject* WorldContextObject);

	/**
	 * 게임 데이터 전체를 단계별로 비동기 로드하는 진입점 함수
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|EDGameDataSubsystem")
    void InitializeGameData();
	
    // 로딩 상태 조회
    UFUNCTION(BlueprintPure, Category = "ED|EDGameDataSubsystem")
    EDataLoadPhase GetCurrentPhase() const { return CurrentPhase; }
	
	// 전체 데이터가 준비되었는지 확인
    UFUNCTION(BlueprintPure, Category = "ED|EDGameDataSubsystem")
    bool IsDataReady() const { return CurrentPhase == EDataLoadPhase::Completed; }
public:
	// ================================================================
	// 언로드
	// ================================================================
	
	// 전체 언로드
	UFUNCTION(BlueprintCallable, Category = "ED|EDGameDataSubsystem")
	void UnloadAllData();
public:
    // ================================================================
    // 델리게이트
    // ================================================================

	/**
	 * 로비 전용 완료 델리게이트, 로비 진입 신호
	 */
	UPROPERTY(BlueprintAssignable, Category = "ED|EDGameDataSubsystem")
	FOnAllLobbyDataLoaded OnLobbyDataReady; 
	
    /** 
     * 모든 데이터 로드 완료 시 방송, 게임 시작 신호 
	 */
    UPROPERTY(BlueprintAssignable, Category = "ED|EDGameDataSubsystem")
    FOnAllDataLoaded OnAllDataLoaded;

    /** 
     * 단계가 바뀔 때마다 방송 -> 로딩 화면있으면 텍스트 변경용으로도 사용가능 
     */
    UPROPERTY(BlueprintAssignable, Category = "ED|EDGameDataSubsystem")
    FOnDataPhaseChanged OnPhaseChanged;
	
    // ================================================================
    // Asset Type 상수 (Project Settings > Asset Manager 등록 이름과 일치)
    // ================================================================
    static const FPrimaryAssetType LobbyAssetType;
    static const FPrimaryAssetType ItemAssetType;
    static const FPrimaryAssetType MonsterAssetType;
private:
	// ================================================================
	// 단계별 로드 함수
	// ================================================================
	void LoadPhase_Lobby();
	void LoadPhase_Item();
	void LoadPhase_Monster();

	// 각 단계 완료 콜백
	void OnLobbyDataLoaded();
	void OnItemDataLoaded();
	void OnMonsterDataLoaded();

	// 공통 캐싱 헬퍼 -> 특정 타입의 로드된 에셋들을 DataCache에 저장
	void CacheLoadedAssets(const FPrimaryAssetType& AssetType);

	// 상태 전이 헬퍼
	void SetPhase(EDataLoadPhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "ED|EDGameDataSubsystem")
	void UnloadPhaseData(const FPrimaryAssetType& AssetType, const FName& HandleKey);
	/**
	 * 에셋 캐시
	 * Key: FPrimaryAssetId
	 * Value: 로드된 UObject* 포인터
	 */
	UPROPERTY()
	TMap<FPrimaryAssetId, TObjectPtr<UObject>> DataCache;

	/**
	 * 핸들 컨테이너 -> 멤버변수로 관리 소멸 방지
	 * Key: 단계 이름
	 */
	TMap<FName, TSharedPtr<FStreamableHandle>> PhaseHandles;

	// ================================================================
	// 내부 상태
	// ================================================================
	EDataLoadPhase CurrentPhase = EDataLoadPhase::NotStarted;
};

// ================================================================
// 템플릿 구현부
// ================================================================

template<typename T>
T* UEDGameDataSubsystem::GetData(const FPrimaryAssetId& Id) const
{
	const TObjectPtr<UObject>* Found = DataCache.Find(Id);
	return Found ? Cast<T>(*Found) : nullptr;
}

template<typename T>
TArray<T*> UEDGameDataSubsystem::GetAllDataOfType(const FPrimaryAssetType& AssetType) const
{
	TArray<T*> Result;

	for (const auto& Pair : DataCache)
	{
		if (Pair.Key.PrimaryAssetType == AssetType)
		{
			if (T* Casted = Cast<T>(Pair.Value.Get()))
			{
				Result.Add(Casted);
			}
		}
	}
	return Result;
}