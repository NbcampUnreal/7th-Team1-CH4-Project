// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/GAS/GA_MonsterSkillAbility.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

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
	// 쿨타임 적용
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);
	// AnimNotify 이벤트 대기 = 몽타주 중간에 폭발트리거
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FEDGameplayTags::Get().Event_Monster_RageExplosion);
	EventTask->EventReceived.AddDynamic(this, &UGA_MonsterSkillAbility::OnExplosionEvent);
	EventTask->ReadyForActivation();
	
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

void UGA_MonsterSkillAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (IsValid(CooldownEffectClass) == false)
		return;
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (IsValid(ASC) == false)
		return;
	
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CooldownEffectClass, 1.f, Context);
	if (SpecHandle.IsValid())
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void UGA_MonsterSkillAbility::OnMontageComplete()
{
	UE_LOG(LogTemp, Warning, TEXT("[MonsterSkillAbility] OnMontageCompleted 호출"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterSkillAbility::OnExplosionEvent(FGameplayEventData Payload)
{
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(CurrentActorInfo->AvatarActor.Get());
	if (IsValid(Monster) == false)
		return;
	
	ApplyExplosionDamage(Monster);
	if (IsValid(ExplosionEffect))
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(Monster, ExplosionEffect, Monster->GetActorLocation());
}

void UGA_MonsterSkillAbility::ApplyExplosionDamage(AEDMonsterBase* Monster)
{
	if (IsValid(DamageEffectClass) == false)
		return;
	
	UAbilitySystemComponent* MonsterASC = Monster->GetAbilitySystemComponent();
	if (IsValid(MonsterASC) == false)
		return;
	
	TArray<AActor*> OverlapActors;
	TArray<AActor*> ActorsToIgnore { Monster };
	UKismetSystemLibrary::SphereOverlapActors(
		Monster, Monster->GetActorLocation(), ExplosionRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		nullptr, ActorsToIgnore, OverlapActors);
	
	const float Atk = IsValid(Monster->GetDataAsset()) ? Monster->GetDataAsset()->GetStat().Atk : 0.f;
	const float DamageAmount = Atk * 1.5f;
	
	for (AActor* HitActor : OverlapActors)
	{
		IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HitActor);
		if (TargetASI == nullptr)
			continue;

		UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
		if (IsValid(TargetASC) == false)
			continue;
		
		if (Cast<AEDMonsterBase>(HitActor))
			continue;
		
		FGameplayEffectContextHandle Context = MonsterASC->MakeEffectContext();
		Context.AddSourceObject(Monster);
		FGameplayEffectSpecHandle  SpecHandle = MonsterASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
		if (SpecHandle.IsValid() == false)
			continue;
		SpecHandle.Data->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_Damage, DamageAmount);
		MonsterASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		
	}
}
