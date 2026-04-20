// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EDSkillDataSubsystem.generated.h"


/**
 * 
 */
USTRUCT(BlueprintType)
struct FSkillMulStatus
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float DamageStrengthMultiplier = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float DamageDexterityMultiplier = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	float DamageIntelligenceMultiplier = 0.f;
};


UCLASS()
class ETERNALDREAMS_API UEDSkillDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	static const UEDSkillDataSubsystem* Get(UWorld* World);
	
	
	// 특정 스킬 태그로 데이터를 빠르게 검색
	const FSkillMulStatus* GetSkillData(FGameplayTag SkillTag) const;

private:
	
	TMap<FGameplayTag, FSkillMulStatus> SkillMulMap;
	
};
