// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "EDAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
	virtual void StartInitialLoading() override;	
};
