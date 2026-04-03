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
				TEXT("[EDAssetManager] LoadAssetsSync - 유효하지 않은 경로 건너뜀 : %s"), *AssetPath.ToString());
			OutAssets.Add(nullptr);
			continue;
		}
		
		UObject* Loaded = GetStreamableManager().LoadSynchronous(AssetPath, false);
		
		if (!Loaded)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[EDAssetManager] LoadAssetsSync - 에셋 로드 실패 : %s"), *AssetPath.ToString());
		}
		OutAssets.Add(Loaded);
	}
}



