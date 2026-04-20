// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/GAS/GA_MonsterSkillAbility.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_MonsterSkillAbility::UGA_MonsterSkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// AssetTag등록
	FGameplayTagContainer Tags;
	Tags.AddTag(FEDGameplayTags::Get().Ability_Monster_Skill);
	SetAssetTags(Tags);
}

void UGA_MonsterSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	
	if (IsValid(SkillMontage) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SkillMontage가 없습니다."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, TEXT("Skill"), SkillMontage);
	if (IsValid(MontageTask) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// 델리게이트 추가 및 실행
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterSkillAbility::OnMontageComplete);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MonsterSkillAbility::OnMontageComplete);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterSkillAbility::OnMontageComplete);
	MontageTask->ReadyForActivation();
}

void UGA_MonsterSkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(ActorInfo->AvatarActor.Get());
	if (IsValid(Monster))
		Monster->OnAttackFinished.Broadcast();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MonsterSkillAbility::OnMontageComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("[MonsterSkillAbility] OnMontageCompleted 호출"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
