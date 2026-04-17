// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/Player/GAS/GameplayAbility/GA_Base.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Data/GameplayTag/EDGameplayTags.h"

UGA_Base::UGA_Base()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGA_Base::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	//Commit
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		//Commit Failed
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	//Play Montage Task(비동기)
	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
	UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
	this,
	NAME_None,
	AnimMontage,
	1.0f
	);
	if (!PlayMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	//Bind Delegate
	PlayMontageTask->OnCompleted.AddDynamic(this, &UGA_Base::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UGA_Base::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UGA_Base::OnMontageCancelled);
	// Task 활성화
	PlayMontageTask->ReadyForActivation();
	
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
	
}

void UGA_Base::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Base::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Base::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

