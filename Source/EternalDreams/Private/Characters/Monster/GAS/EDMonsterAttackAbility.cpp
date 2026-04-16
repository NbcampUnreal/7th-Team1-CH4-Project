// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/GAS/EDMonsterAttackAbility.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UEDMonsterAttackAbility::UEDMonsterAttackAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// TryActivateAbilitiesByTag를 위한 AssetTag 등록
	FGameplayTagContainer Tags;
	Tags.AddTag(FEDGameplayTags::Get().Ability_Monster_Attack);
	SetAssetTags(Tags);
}

void UEDMonsterAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(ActorInfo->AvatarActor.Get());
	if (IsValid(Monster) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (IsValid(AttackMontage) == false)
	{
		UE_LOG(LogTemp,Warning,TEXT("[%s] AttackMontage가 없습니다."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, TEXT("Attack"), AttackMontage);
	if (IsValid(MontageTask) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	MontageTask->OnCompleted.AddDynamic(this, &UEDMonsterAttackAbility::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UEDMonsterAttackAbility::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UEDMonsterAttackAbility::OnMontageCompleted);
	MontageTask->ReadyForActivation();
}

void UEDMonsterAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(ActorInfo->AvatarActor.Get());
	if (IsValid(Monster))
		Monster->OnAttackFinished.Broadcast();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UEDMonsterAttackAbility::OnMontageCompleted()
{
	UE_LOG(LogTemp, Warning, TEXT("[MonsterAttackAbility] OnMontageCompleted 호출"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}