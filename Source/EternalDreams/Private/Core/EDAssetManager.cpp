// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/EDAssetManager.h"

UEDAssetManager::UEDAssetManager()
{
}

UEDAssetManager& UEDAssetManager::Get()
{
	check(GEngine);
	
	if (UEDAssetManager* Singleton = Cast<UEDAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}
	
	UE_LOG(LogTemp, Fatal, TEXT("Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to EDAssetManager!"));
	
	return *NewObject<UEDAssetManager>();
}

/**
 * Super::StartInitialLoading() 
 * Super호출해서 DefaultEngine.ini or Project Settings의 Primary Asset 타입/경로 읽기
 */
void UEDAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	// TODO : 프로젝트 커스텀 초기화 작성
	UE_LOG(LogTemp, Log, TEXT("[EDAssetManager] StartInitialLoading 실행완료"));
}

// 경로 배열을 받아서 각 경로의 에셋을 순서대로 동기 로드 -> OutAssets배열에 담음
void UEDAssetManager::LoadAssetsSync(const TArray<FSoftObjectPath>& AssetPaths, TArray<UObject*>& OutAssets)
{
	// OutAssets 비우면서 TArray길이만큼 재할당
	OutAssets.Empty(AssetPaths.Num());
	
	for (const FSoftObjectPath& AssetPath : AssetPaths)
	{
		if (!AssetPath.IsValid())
		{
			UE_LOG(LogTemp, Warning, 
				TEXT("[EDAssetManager] LoadAssetsSync - 유효하지 않은 경로 건너뜀 : %s"), 
				*AssetPath.ToString());
			
			OutAssets.Add(nullptr);
			continue;
		}
		
		UObject* Loaded = GetStreamableManager().LoadSynchronous(AssetPath, false);
		
		if (!Loaded)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[EDAssetManager] LoadAssetsSync - 에셋 로드 실패 : %s"), 
				*AssetPath.ToString());
		}
		OutAssets.Add(Loaded);
	}
}

// 비동기 로드
TSharedPtr<FStreamableHandle> UEDAssetManager::LoadAssetAsync(const FSoftObjectPath& AssetPath,
	FStreamableDelegate OnLoaded, TAsyncLoadPriority Priority)
{
	if (!AssetPath.IsValid())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[EDAssetManager] LoadAssetAsync - 유효하지 않은 에셋 경로입니다."));
		return nullptr;
	}
	
	/* 
	 * 이미 로드된 에셋 최적화
	 * 현재 메모리에서 에셋 찾음 -> 이미 로드되어있다면 콜백 실행
	 */
	if (AssetPath.ResolveObject() != nullptr)
	{
		UE_LOG(LogTemp, Verbose,
			TEXT("[EDAssetManager] LoadAssetAsync - 이미 로드된 에셋, 콜백 즉시 실행: %s"),
			*AssetPath.ToString());

		// 콜백 실행
		if (OnLoaded.IsBound())
		{
			OnLoaded.Execute();
		}
		
		// 이미 로드된 에셋을 위한 동기 핸들 생성해서 반환해줌
		// 호출자는 핸들을 통해 에셋을 GC로부터 보호가능
		return GetStreamableManager().RequestSyncLoad(AssetPath);
	}
	
	/* 
	 * 비동기 로드 요청
	 * AssetPath : 에셋 경로
	 * OnLoaded : 완료 콜백
	 * Priority : 우선순위
	 * bManagerActiveHandle : false면 핸들 만료시 자동 정리
	 */
	return GetStreamableManager().RequestAsyncLoad(
		AssetPath,
		OnLoaded,
		Priority,
		false
		);
}

TSharedPtr<FStreamableHandle> UEDAssetManager::LoadAssetsAsync(
	const TArray<FSoftObjectPath>& AssetPaths,
	FStreamableDelegate OnAllLoaded,
	TAsyncLoadPriority Priority)
{
	if (AssetPaths.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EDAssetManager] LoadAssetsAsync - 에셋 경로 배열이 비어있습니다."));
		return nullptr;
	}
	
	TArray<FSoftObjectPath> ValidPaths;
	ValidPaths.Reserve(AssetPaths.Num());
	
	for (const FSoftObjectPath& AssetPath : AssetPaths)
	{
		if (AssetPath.IsValid())
		{
			ValidPaths.Add(AssetPath);
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[EDAssetManager] LoadAssetsAsync - 경로가 유효하지 않습니다. 경로 : %s"), 
				*AssetPath.ToString());
		}
	}
	
	if (ValidPaths.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[EDAssetManager] LoadAssetsAsync - 유효한 경로가 없습니다."));
		return nullptr;
	}
	
	/**
	 * 다중 에셋 비동기 로드
	 * 여러 경로를 배열로 넘기면 단일 핸들로 묶어서 처리
	 * 모든 에셋처리가 끝나면 OnAllLoaded 콜백 호출
	 */
	return GetStreamableManager().RequestAsyncLoad(
		ValidPaths,
		OnAllLoaded,
		Priority,
		false);
}



