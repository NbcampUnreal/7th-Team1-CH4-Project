// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Base/GAS/GA_Death.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Data/GameplayTag/EDGameplayTags.h"

UGA_Death::UGA_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// 사망 어빌리티 활성화 중 State.Dead 태그 부여
	ActivationOwnedTags.AddTag(FEDGameplayTags::Get().State_Dead);
	// TryActivateAbilitiesByTag를 위한 AssetTag 등록
	FGameplayTagContainer Tags;
	Tags.AddTag(FEDGameplayTags::Get().State_Dead);
	SetAssetTags(Tags);
}

void UGA_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (IsValid(DeathMontage) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	// MontageTask 생성(비동기)
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, TEXT("Death"), DeathMontage);
	if (IsValid(MontageTask) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	// Montage 완료, 중단시 콜백 
	MontageTask->OnCompleted.AddDynamic(this, &UGA_Death::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_Death::OnMontageCompleted);
	// Montage 준비 완료 및 실행
	MontageTask->ReadyForActivation();
	
}

void UGA_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Death::OnMontageCompleted()
{
	// Montage 완료 후 Ability 종료 몽타주 마지막 상태로 유지
	//EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	
}
