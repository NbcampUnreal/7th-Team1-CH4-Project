// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EDPlayerAnimDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDPlayerAnimDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(GetItemType(), GetFName());
	}
	
	virtual FPrimaryAssetType GetItemType() const
	{
		return FPrimaryAssetType("PlayerAnimData");
	}
	
public:
	//애님몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Data", meta=(AssetBundles="PlayerData"))
	TSoftObjectPtr<UAnimMontage> AnimMontage;
};
