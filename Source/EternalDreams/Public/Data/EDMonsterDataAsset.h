// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/EDMonsterTypes.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "EDMonsterDataAsset.generated.h"


/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDMonsterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FText GetMonsterName() const { return MonsterName; }
	UFUNCTION(BlueprintCallable)
	EMonsterGrade GetGrade() const { return Grade; }

	UFUNCTION(BlueprintCallable)
	FMonsterStatRow GetStat() const { return StatData; }

	UFUNCTION(BlueprintCallable)
	virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(TEXT("MonsterData"), GetFName()); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	FText MonsterName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	EMonsterGrade Grade;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	FMonsterStatRow StatData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TSoftObjectPtr<USkeletalMesh> Mesh;
	// 임시 -> 추후 EDMonsterAnimInstance로 교체 예정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TSoftClassPtr<UAnimInstance> AnimInstance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;
	// TODO
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	// TSubclassOf<UEDMonsterBase> MonsterClass;
};
