// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Core/EDSkillDataSubsystem.h"
#include "EDSkillMulTypeRow.generated.h"

/**
 * 스킬의 GamePlayTag와 계수를 나타내는 DT Row
 */
USTRUCT(BlueprintType)
struct FEDSkillMulTypeRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	FGameplayTag SkillTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FSkillMulStatus SkillMultiplier;
};