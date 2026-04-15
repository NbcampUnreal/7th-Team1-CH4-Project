// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/GAS/GameplayAbility/GA_SelfEffect.h"

#include "AbilitySystemComponent.h"
#include "SNegativeActionButton.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AI/NavigationModifier.h"
#include "Data/GameplayTag/EDGameplayTags.h"

void UGA_SelfEffect::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	//Commit
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		//Commit Failed
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	ApplyEffect();
	EndAbility(Handle,ActorInfo,ActivationInfo,true,false);
}

void UGA_SelfEffect::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SelfEffect::ApplyEffect()
{
	if (!SelfEffect)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	//EffectContext 생성
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo());

	//EffectSpec 생성
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(
		SelfEffect, 1.0f, Context);

	if (Spec.IsValid())
	{
		FGameplayTag DataTag = FEDGameplayTags::Get().Data_StatMul;
		if (!DataTag.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("태그가 유효하지 않습니다! DefaultGameplayTags.ini를 확인하세요."));
		}
		Spec.Data.Get()->SetSetByCallerMagnitude(DataTag, MulPercent);
		UE_LOG(LogTemp,Warning,TEXT("Mulpercent %f"),MulPercent);
		
		float FoundMagnitude = Spec.Data.Get()->GetSetByCallerMagnitude(DataTag, false);
		UE_LOG(LogTemp, Warning, TEXT("Confirmed Magnitude in Spec: %f"), FoundMagnitude);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

