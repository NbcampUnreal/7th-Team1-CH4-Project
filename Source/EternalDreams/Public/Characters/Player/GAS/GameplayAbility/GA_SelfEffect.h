// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GA_Base.h"
#include "GA_SelfEffect.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UGA_SelfEffect : public UGA_Base
{
	GENERATED_BODY()
	
public:
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
	
	void ApplyEffect();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "SelfEffect")
	TSubclassOf<UGameplayEffect> SelfEffect;
	
	UPROPERTY(EditAnywhere, Category = "SelfEffect")
	float MulPercent=1.5f;
	
	
	
	

};
