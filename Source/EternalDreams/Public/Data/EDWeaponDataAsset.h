// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EDWeaponDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(GetItemType(), GetFName());
	}
	
	virtual FPrimaryAssetType GetItemType() const
	{
		return FPrimaryAssetType("WeaponData");
	}
	
public:
	//StaticMesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon",meta=(AssetBundles="PlayerData"))
	TSoftObjectPtr<UStaticMesh> WeaponStaticMesh;
	
};
