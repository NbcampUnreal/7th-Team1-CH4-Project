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
	 * 에셋이 이미 메모리에 있다 -> OnLoaded 즉시 호출
	 * 메모리에 없다 -> 백그라운드에서 I/O수행 -> 완료 시 OnLoaded콜백 호출
	 * 
	 * @param AssetPath		로드할 에셋의 소프트 경로
	 * @param OnLoaded		로드 완료 시 호출될 FStreamableDelegate 콜백
	 * @param Priority		우선순위 (높을수록 먼저 로드)
	 * 
	 * @return TSharedPtr<FStreamableHandle> 멤버 변수로 유지하기 (GC에게 제거당하지 않기위해), 유효하지 않은 경로면 nullptr 반환
	 */
	TSharedPtr<FStreamableHandle> LoadAssetAsync(
		const FSoftObjectPath& AssetPath,
		FStreamableDelegate OnLoaded = FStreamableDelegate(),
		TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority
		);
	
	
	/**
	 * [비동기] FSoftObjectPath 배열의 여러 에셋을 비동기로 한꺼번에 로드
	 * @param AssetPaths	로드할 에셋 경로 배열 (유효하지 않은 경로는 자동으로 필터링)
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
	 *       FStreamableDelegate::CreateUObject(this, &AMyActor::OnItemDataLoaded));
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
