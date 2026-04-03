// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "EDAssetManager.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class ETERNALDREAMS_API UEDAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	UEDAssetManager();
	
	/**
	 * 전역 UEDAssetManager 싱글톤 인스턴스 반환
	 * ex) UEDAssetManager& Manager = UEDAssetManager::Get();
	 */ 
	static UEDAssetManager& Get();
protected:
	/**
	 * 엔진 시작시 에셋 매니저 초기화할때 자동 호출
	 */
	virtual void StartInitialLoading() override;
public:
	// ================================================================
	// 동기 (Sync)
	// ================================================================
	
	/**
	* [동기] FSoftObjectPath로 에셋을 즉시 로드 타입 캐스팅한 포인터를 반환
	* 
	* 에셋이 메모리에 있으면 -> 즉시 캐스팅 후 반환
	* 메모리에 없으면 -> FStreamableManager::LoadSynchronous 호출
	* 
	* @tparam AssetType		반환받을 에셋 타입, ex) UStaticMesh
	* @param AssetPath		로드할 에셋의 소프트 경로, ex) FSoftObjectPath("경로")
	*
	* 
	* @return				로드 성공 : AssetType*, 실패 : nullptr
	* 사용 예 
	* UStaticMesh* Mesh = UEDAssetManager::Get()
	*			.LoadAssetSync<UStaticMesh>(FSoftObjectPath("메쉬경로"));
	*/
	template<typename AssetType>
	AssetType* LoadAssetSync(const FSoftObjectPath& AssetPath);

	/**
	 * [동기] FPrimaryAssetId로 Primary Asset을 즉시 로드해서 반환
	 * 
	 * GetPrimaryAssetPath()로 FPrimaryAssetId -> FSoftObjectPath 반환
	 * LoadAssetSync<AssetType>()호출
	 * 
	 * @tparam AssetType		반환받을 에셋 타입
	 * @param PrimaryAssetId	로드할 에셋의 Primary Asset Id, ex) FPrimaryAssetId("MonsterData", "Monster_001"))
	 * @return					로드 성공 : AssetType* 포인터, 실패 : nullptr 
	 * 사용 예
	 * UMonsterData* MonsterData = UEDAssetManager::Get()
	 *       .LoadPrimaryAssetSync<UMonsterData>(FPrimaryAssetId("MonsterData", "Monster_001"));
	 */
	template<typename AssetType>
	AssetType* LoadPrimaryAssetSync(const FPrimaryAssetId& PrimaryAssetId);


	/**
	 * [동기] 여러 에셋 경로 TArray로 받아와서 한꺼번에 동기 로드
	 * 
	 * 에셋 수가 많을수록 블로킹 시간이 길어서 런타임에 절대 실행 금지 XXX
	 * 
	 * @param AssetPaths	로드할 에셋 경로 배열 
	 * @param OutAssets		로드된 UObject 포인터 배열
	 * 
	 * 사용 예
	 * TArray<FSoftObjectPath> Paths = {Path1, Path2, Path3};
	 * TArray<UObject*> Loaded;
	 * UEDAssetManager::Get().LoadAssetSync(Paths, Loaded);
	 * 
	 */
	void LoadAssetsSync(const TArray<FSoftObjectPath>& AssetPaths, TArray<UObject*>& OutAssets);
	
public:
	// ================================================================
	// 비동기 (Async)
	// ================================================================
	
	/**
	 * [비동기] FSoftObjectPath 단일 에셋을 비동기로 로드 
	 * 
	 * 에셋이 이미 메모리에 있다 -> OnLoaded 호출
	 * 메모리에 없다 -> 백그라운드에서 I/O수행 -> 완료 시 OnLoaded콜백 호출
	 * 
	 * @param AssetPath		로드할 에셋의 소프트 경로
	 * @param OnLoaded		로드 완료 시 호출될 FStreamableDelegate 콜백
	 * @param Priority		우선순위 (높을수록 먼저 로드)
	 * 
	 * @return TSharedPtr<FStreamableHandle> -> 호출한 클래스에서 멤버 변수로 유지하기
	 */
	TSharedPtr<FStreamableHandle> LoadAssetAsync(
		const FSoftObjectPath& AssetPath,
		FStreamableDelegate OnLoaded = FStreamableDelegate(),
		TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority
		);
	
	
	/**
	 * [비동기] FSoftObjectPath 배열의 여러 에셋을 비동기로 한꺼번에 로드
	 * @param AssetPaths	로드할 에셋 경로 배열
	 * @param OnAllLoaded	모든 에셋 로드 완료 시 호출될 콜백
	 * @param Priority		스트리밍 우선순위
	 * 
	 * @return	TSharedPtr<FStreamableHandle>, 경로배열 비어있으면 nullptr 반환
	 */
	TSharedPtr<FStreamableHandle> LoadAssetsAsync(
		const TArray<FSoftObjectPath>& AssetPaths,
		FStreamableDelegate            OnAllLoaded = FStreamableDelegate(),
		TAsyncLoadPriority             Priority    = FStreamableManager::DefaultAsyncLoadPriority
	);
	
	
	/**
	 * [비동기] FPrimaryAssetId 단일 Primary Asset을 비동기로 로드
	 *
	 * UAssetManager::LoadPrimaryAsset()을 래핑한 함수, 핸들 반환하여 에셋 수명관리를 직접 할 수 있음
	 * AssetBundle을 이용해 필요한 Secondary Asset만 부분 로드
	 * 
	 * @param PrimaryAssetId    로드할 Primary Asset ID
	 * @param Bundles           함께 로드할 Asset Bundle 이름 배열
	 *                          예시: { TEXT("UI") }, { TEXT("ITEM") })
	 *                          빈 배열이면 Primary Asset 자체만 로드
	 * @param OnLoaded          로드 완료 시 호출될 콜백
	 * @param Priority          스트리밍 우선순위
	 * 
	 * @return					TSharedPtr<FStreamableHandle>
	 * 
	 * 사용 예
	 * 아이템 UI 표시용 로드 (아이콘, 이름 데이터만 필요)
	 * TArray<FName> UIBundles = { TEXT("UI") };
	 *   ItemHandle = UEDAssetManager::Get().LoadPrimaryAssetAsync(
	 *       FPrimaryAssetId("ItemData", "Sword_001"),
	 *       UIBundles,
	 *       FStreamableDelegate::CreateUObject(this, &AMyActor::OnItemDataLoaded)
	 *       );
	 *   void AMyActor::OnItemDataLoaded()
	 *   {
	 *       UMyItemData* Item = UEDAssetManager::Get()
	 *           .GetPrimaryAsset<UMyItemData>(FPrimaryAssetId("ItemData", "Sword_001"));
	 *   }
	 *   );
	 */
	TSharedPtr<FStreamableHandle> LoadPrimaryAssetAsync(
		const FPrimaryAssetId&  PrimaryAssetId,
		const TArray<FName>&    Bundles  = TArray<FName>(),
		FStreamableDelegate     OnLoaded = FStreamableDelegate(),
		TAsyncLoadPriority      Priority = FStreamableManager::DefaultAsyncLoadPriority
	);
	
	/**
	* [비동기] FPrimaryAssetId 배열의 여러 Primary Asset을 비동기로 한꺼번에 로드
	*
	* 모든 Primary Asset 및 지정된 Bundle의 Secondary Asset이 완료된 후 OnAllLoaded 콜백이 단 한 번 호출
	*
	* @param PrimaryAssetIds   로드할 Primary Asset ID 배열
	* @param Bundles           함께 로드할 Bundle 이름 배열 (모든 ID에 동일 적용)
	* @param OnAllLoaded       전체 완료 시 호출될 콜백
	* @param Priority          스트리밍 우선순위
	* @return                  TSharedPtr<FStreamableHandle>
	*
	* 사용 예
	*   TArray<FPrimaryAssetId> Ids = {
	*       FPrimaryAssetId("Monster", "Chicken"),
	*       FPrimaryAssetId("Monster", "Wolf"),
	*       FPrimaryAssetId("Monster", "Dog"),
	*   };
	*   TArray<FName> InGameBundle = { TEXT("InGame") };
	*   MonsterHandle = UEDAssetManager::Get().LoadPrimaryAssetsAsync(
	*       Ids, InGameBundle,
	*       FStreamableDelegate::CreateUObject(this, &AMyGameMode::OnMonstersLoaded)
	*   );
	*/
	TSharedPtr<FStreamableHandle> LoadPrimaryAssetsAsync(
		const TArray<FPrimaryAssetId>& PrimaryAssetIds,
		const TArray<FName>&           Bundles      = TArray<FName>(),
		FStreamableDelegate            OnAllLoaded  = FStreamableDelegate(),
		TAsyncLoadPriority             Priority     = FStreamableManager::DefaultAsyncLoadPriority
	);
	
	
public:
	// ================================================================
	// 유틸리티 (Utility)
	// ================================================================
	
	/**
	* 이미 로드된 Primary Asset을 지정 타입으로 캐스팅하여 반환합니다.
	*
	* 반드시 LoadPrimaryAssetAsync / LoadPrimaryAssetSync 호출 후, 로드 완료(콜백) 시점에서 호출
	* 
	* @tparam AssetType      반환받을 에셋 타입
	* @param  PrimaryAssetId 가져올 Primary Asset ID
	* @return                로드된 에셋 포인터. 로드 전 또는 실패 시 nullptr
	* 
	* 사용 예
	* 비동기 콜백안에서 호출
	*   void AMyActor::OnItemDataLoaded()
	*   {
	*       UMyItemData* Item = UEDAssetManager::Get()
	*           .GetPrimaryAsset<UMyItemData>(FPrimaryAssetId("ItemData", "Sword_001"));
	*       if (Item) { ItemName = Item->DisplayName; }
	*   }
	* 
	*/
	template<typename AssetType>
	AssetType* GetPrimaryAsset(const FPrimaryAssetId& PrimaryAssetId);
	
	/**
	 * Primary Asset이 현재 메모리에 로드되어 있는지 여부를 반환
	 *
	 * 비동기 로드 완료 전 상태 확인에 활용가능
	 *
	 * @param PrimaryAssetId    확인할 Primary Asset ID
	 * @return                  메모리에 로드되어 있으면 true, 아니면 false
	 */
	bool IsPrimaryAssetLoaded(const FPrimaryAssetId& PrimaryAssetId) const;
};

// ================================================================
// 템플릿 함수 구현
// ================================================================

template <typename AssetType>
AssetType* UEDAssetManager::LoadAssetSync(const FSoftObjectPath& AssetPath)
{
	// 경로 유효성 검사
	if (!AssetPath.IsValid())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[EDAssetManager] LoadAssetSync - 유효하지 않은 에셋 경로입니다."));
		return nullptr;
	}

	/**
	 * FStreamableManager를 통한 동기 로드
	 * LoadSynchronous 내부 동작:
	 * 이미 로드된 경우  → 바로 UObject* 반환
	 * 로드 안되어 있던 경우 → 비동기 I/O 요청 → WaitUntilComplete()로 블로킹 대기
	 *
	 * 두 번째 인자 bErrorIfNotFound: false → 없어도 에러 없이 nullptr 반환
	 */
	UObject* LoadedObject = GetStreamableManager().LoadSynchronous(AssetPath, false);

	if (!LoadedObject)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[EDAssetManager] LoadAssetSync - 에셋 로드 실패: %s"), *AssetPath.ToString());
		return nullptr;
	}

	// 요청한 타입으로 캐스팅하여 반환, 타입안맞으면 nullptr
	return Cast<AssetType>(LoadedObject);
}

template <typename AssetType>
AssetType* UEDAssetManager::LoadPrimaryAssetSync(const FPrimaryAssetId& PrimaryAssetId)
{
	// ID 유효성 검사
	if (!PrimaryAssetId.IsValid())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[EDAssetManager] LoadPrimaryAssetSync - 유효하지 않은 PrimaryAssetId입니다."));
		return nullptr;
	}

	/** PrimaryAssetId → FSoftObjectPath 변환
	 *  GetPrimaryAssetPath()는 Asset Manager 내부 등록 테이블을 조회
	 *  Project Settings -> Asset Manager에 해당 타입이 등록되지 않았거나
	 *  스캔 경로에 에셋이 없으면 빈 경로가 반환 IsValid로 검사
	 */
	FSoftObjectPath AssetPath = GetPrimaryAssetPath(PrimaryAssetId);

	if (!AssetPath.IsValid())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[EDAssetManager] LoadPrimaryAssetSync - 경로 조회 실패 ID: %s"),
			*PrimaryAssetId.ToString());
		return nullptr;
	}

	// 경로를 사용해 동기 로드
	return LoadAssetSync<AssetType>(AssetPath);
}

template <typename AssetType>
AssetType* UEDAssetManager::GetPrimaryAsset(const FPrimaryAssetId& PrimaryAssetId)
{
	/* GetPrimaryAssetObject() 는 Asset Manager 내부 맵에서
	* 현재 로드된 UObject*를 찾아 반환
	* 로드되지 않은 에셋이면 nullptr이 반환
	*/
	UObject* FoundObject = GetPrimaryAssetObject(PrimaryAssetId);
	return Cast<AssetType>(FoundObject);
}
