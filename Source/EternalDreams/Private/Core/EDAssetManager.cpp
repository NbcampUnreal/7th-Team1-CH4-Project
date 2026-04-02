// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/EDAssetManager.h"
#include "AbilitySystemGlobals.h"

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
}
