// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/GAS/EDMonsterAttackAbility.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Data/EDMonsterDataAsset.h"
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
	
	// TODO: AnimNotify 기반으로 교체 예정(현재는 공격 시작 시 즉시 적용)
	AAIController* AIController = Cast<AAIController>(Monster->GetController());
	if (IsValid(AIController) == false)
		return;
	
	AActor* Target = AIController->GetFocusActor();
	if (IsValid(Target) == false)
		return;
	
	IAbilitySystemInterface* TargetASCInterface = Cast<IAbilitySystemInterface>(Target);
	if (TargetASCInterface == nullptr)
		return;
	
	UAbilitySystemComponent* TargetASC = TargetASCInterface->GetAbilitySystemComponent();
	if (IsValid(TargetASC) == false)
		return;
	// 몬스터 Atk가 AttributeSet에 있지 않기 때문에 DataAsset에서 불러와서 Atk선언
	const float Atk = IsValid(Monster->GetDataAsset()) ? Monster->GetDataAsset()->GetStat().Atk : 0.0f;
	// 레벨 1로 DamageEffect 스펙 생성
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, 1.f);
	if (SpecHandle.IsValid() == false)
		return;
	// SpecHandle의 데이터중 데미지에 -Atk를 넣어서 플레이어의 HP에 Add할 Data 생성
	SpecHandle.Data->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_Damage, -Atk);
	// Monster ASC -> Player ASC 데미지 GE 적용
	ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(
		*SpecHandle.Data.Get(), TargetASC);
	
	// 데미지 적용 후 플레이어 HP 로그
	float CurrentHP = TargetASC->GetNumericAttribute(UEDBaseAttributeSet::GetHealthAttribute());
	UE_LOG(LogTemp, Warning, TEXT("[MonsterAttack] 플레이어 HP: %.1f"), CurrentHP);
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

// void UEDMonsterAttackAbility::PostInitProperties()
// {
// 	Super::PostInitProperties();
// 	// BTTask_MonsterAttack에서 이 태그로 활성화
// 	FGameplayTagContainer Tags;
// 	Tags.AddTag(FEDGameplayTags::Get().Ability_Monster_Attack);
// 	SetAssetTags(Tags);
// }
