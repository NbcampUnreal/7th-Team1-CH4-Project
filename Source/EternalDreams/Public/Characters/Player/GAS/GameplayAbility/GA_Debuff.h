// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/Types/EDPlayerTypes.h"
#include "GA_Debuff.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UGA_Debuff : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Debuff();
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayTagContainer* SourceTags = nullptr, 
		const FGameplayTagContainer* TargetTags = nullptr, 
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;
	
	virtual void ActivateAbility(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,
	   const FGameplayEventData* TriggerEventData
   ) override;
	
	virtual void EndAbility(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,
	   bool bReplicateEndAbility,
	   bool bWasCancelled
   ) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	EPlayerAnimNameType MontageName=EPlayerAnimNameType::NONE;
	
	UPROPERTY()
	UAnimMontage* PlayerAnimMontage=nullptr;
	
	UFUNCTION()
	void OnDebuffTagRemoved();
};
