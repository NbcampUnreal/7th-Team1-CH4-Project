// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "EDMonsterAttackAbility.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDMonsterAttackAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UEDMonsterAttackAbility();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility, 
	bool bWasCancelled) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage;
	
private:
	UFUNCTION()
	void OnMontageCompleted();
	
	virtual void PostInitProperties() override;
};
