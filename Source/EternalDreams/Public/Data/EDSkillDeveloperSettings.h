// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EDSkillDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Skill Settings"))
class ETERNALDREAMS_API UEDSkillDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	// 프로젝트 세팅에서 노출
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> SkillMulDataTable;
};
