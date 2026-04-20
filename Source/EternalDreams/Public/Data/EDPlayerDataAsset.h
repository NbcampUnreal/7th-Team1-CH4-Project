// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EDPlayerDataAsset.generated.h"


/**
 * Player에 필요한 데이터를 외부에서 주입하는 PDA입니다.
 */



UCLASS()
class ETERNALDREAMS_API UEDPlayerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(GetItemType(), GetFName());
	}
	
	virtual FPrimaryAssetType GetItemType() const
	{
		return FPrimaryAssetType("PlayerData");
	}
	
public:
	//스켈레탈 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Data", meta=(AssetBundles="PlayerData"))
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
	
	//ABP
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Data", meta=(AssetBundles="PlayerData"))
	TSoftClassPtr<UAnimInstance> AnimationBlueprint;
	


};