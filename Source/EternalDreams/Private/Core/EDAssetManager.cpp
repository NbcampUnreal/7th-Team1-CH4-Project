// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/EDAssetManager.h"
#include "Data/GameplayTag/EDGameplayTags.h"

void UEDAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	FEDGameplayTags::InitializeNativeTags();
	//UE_LOG(LogTemp,Warning,TEXT("TagInitialized"));
}
