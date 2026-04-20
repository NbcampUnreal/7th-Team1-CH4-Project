// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EDMonsterAbilityScore.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EAbilityRangeType : uint8
{
	Melee,    // 선호 거리 이하일 때 점수
	Ranged,   // 선호 거리 이상일 때 점수
};

USTRUCT(BlueprintType)
struct FAbilityScoreContext
{
	GENERATED_BODY()
	
	// 활성화할 어빌리티 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	FGameplayTag AbilityTag;
	// Ability 종류 (근접/원거리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	EAbilityRangeType RangeType = EAbilityRangeType::Melee;
	// 거리 조건 기준 값(RangeType에 따라 이하/이상 판단)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	float PreferredRange = 300.f;
	// 거리 조건 충족 시 추가 점수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	float RangeScore = 30.f;
	// Rage 발동 HP 비율 기준값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	float RageHPThreshold = 0.f;
	// RageHPThreshold 충족 시 추가 점수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	float RageScore = 0.f;
	// 기본 점수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	float BaseScore = 50.f;
	// 쿨타임 태그(활성화 중이면 이 어빌리티 스킵)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	FGameplayTag CooldownTag;
};
