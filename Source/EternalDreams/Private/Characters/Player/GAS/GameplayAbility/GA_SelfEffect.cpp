// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/GAS/GameplayAbility/GA_SelfEffect.h"

#include "AbilitySystemComponent.h"

#if WITH_EDITOR
#include "SNegativeActionButton.h"
#endif

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
	
	//GE_CoolDown 적용
	if (CoolTimeEffectClass==nullptr||CoolTime==0.f)
	{
		return;
	}
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CoolTimeEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_CoolTime, CoolTime);
		
		SpecHandle.Data.Get()->DynamicGrantedTags.AddTag(CoolTimeTag);
		
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
	
	
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
			return;
		}
		Spec.Data.Get()->SetSetByCallerMagnitude(DataTag, MulPercent);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

