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
	*/
	template<typename AssetType>
	AssetType* LoadAssetSync(const FSoftObjectPath& AssetPath);

	/**
	 * [동기] FPrimaryAssetId로 Primary Asset을 즉시 로드해서 반환
	 */
	template<typename AssetType>
	AssetType* LoadPrimaryAssetSync(const FPrimaryAssetId& PrimaryAssetId);
	
	/**
	 * [동기] 여러 에셋 경로 TArray로 받아와서 한꺼번에 동기 로드
	 */
	void LoadAssetsSync(const TArray<FSoftObjectPath>& AssetPaths, TArray<UObject*>& OutAssets);
	
public:
	// ================================================================
	// 비동기 (Async)
	// ================================================================
	
	/**
	 * [비동기] FSoftObjectPath 단일 에셋을 비동기로 로드 
	 */
	TSharedPtr<FStreamableHandle> LoadAssetAsync(
		const FSoftObjectPath& AssetPath,
		FStreamableDelegate OnLoaded = FStreamableDelegate(),
		TAsyncLoadPriority Priority = FStreamableManager::DefaultAsyncLoadPriority
		);
	
	
	/**
	 * [비동기] FSoftObjectPath 배열의 여러 에셋을 비동기로 한꺼번에 로드
	 */
	TSharedPtr<FStreamableHandle> LoadAssetsAsync(
		const TArray<FSoftObjectPath>& AssetPaths,
		FStreamableDelegate            OnAllLoaded = FStreamableDelegate(),
		TAsyncLoadPriority             Priority    = FStreamableManager::DefaultAsyncLoadPriority
	);
	
	
	/**
	 * [비동기] FPrimaryAssetId 단일 Primary Asset을 비동기로 로드
	 */
	TSharedPtr<FStreamableHandle> LoadPrimaryAssetAsync(
		const FPrimaryAssetId&  PrimaryAssetId,
		const TArray<FName>&    Bundles  = TArray<FName>(),
		FStreamableDelegate     OnLoaded = FStreamableDelegate(),
		TAsyncLoadPriority      Priority = FStreamableManager::DefaultAsyncLoadPriority
	);
	
	/**
	* [비동기] FPrimaryAssetId 배열의 여러 Primary Asset을 비동기로 한꺼번에 로드
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
	*/
	template<typename AssetType>
	AssetType* GetPrimaryAsset(const FPrimaryAssetId& PrimaryAssetId);
	
	/**
	 * Primary Asset이 현재 메모리에 로드되어 있는지 여부를 반환
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

	/** 
	 * PrimaryAssetId → FSoftObjectPath 변환
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
	UObject* FoundObject = GetPrimaryAssetObject(PrimaryAssetId);
	return Cast<AssetType>(FoundObject);
}
