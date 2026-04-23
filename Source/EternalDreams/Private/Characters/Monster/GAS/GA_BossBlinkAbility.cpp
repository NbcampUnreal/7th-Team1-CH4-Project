// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/GAS/GA_BossBlinkAbility.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemInterface.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

UGA_BossBlinkAbility::UGA_BossBlinkAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	FGameplayTagContainer Tags;
	Tags.AddTag(FEDGameplayTags::Get().Ability_Monster_Blink);
	SetAssetTags(Tags);
}

void UGA_BossBlinkAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	
	AAIController* AIController = Cast<AAIController>(Monster->GetController());
	if (IsValid(AIController) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (IsValid(BB) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (IsValid(Target) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// 쿨타임 적용
	ApplyCooldown(Handle, ActorInfo, ActivationInfo);
	// 타겟 뒤쪽 점멸 도착 지점 계산
	FVector ToTarget = (Target->GetActorLocation() - Monster->GetActorLocation()).GetSafeNormal();
	BlinkDestination = Target->GetActorLocation() + ToTarget * BlinkOffset;
	// 벽 체크 - 도착 지점이 막혀있으면 히트 지점 직전으로 조정
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Monster);
	Params.AddIgnoredActor(Target);
	bool bBlocked = Monster->GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Target->GetActorLocation(),
		BlinkDestination,
		ECC_WorldStatic,
		Params);
	
	if (bBlocked)
		BlinkDestination =  HitResult.Location - ToTarget * 50.f;
	// NavMesh 위로 높이 보정
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Monster->GetWorld());
	FNavLocation NavLocation;
	if (IsValid(NavSys) && NavSys->ProjectPointToNavigation(BlinkDestination, NavLocation))
		BlinkDestination = NavLocation.Location;
	// 캡슐 절반 높이만큼 위로 올려서 바닥 끼임 방지
	BlinkDestination.Z += Monster->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 도착 인디케이터
	if (IsValid(IndicatorEffect))
		Monster->Multicast_SpawnEffect(IndicatorEffect, BlinkDestination);
	// 딜레이 후 점멸 실행
	UAbilityTask_WaitDelay* WaitTask = UAbilityTask_WaitDelay::WaitDelay(this, BlinkDelay);
	WaitTask->OnFinish.AddDynamic(this, &UGA_BossBlinkAbility::OnBlinkDelayFinished);
	WaitTask->ReadyForActivation();
}

void UGA_BossBlinkAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(ActorInfo->AvatarActor.Get());
	if (IsValid(Monster))
		Monster->OnAttackFinished.Broadcast();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_BossBlinkAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
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

void UGA_BossBlinkAbility::OnBlinkDelayFinished()
{
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(CurrentActorInfo->AvatarActor.Get());
	if (IsValid(Monster) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	AAIController* AIController = Cast<AAIController>(Monster->GetController());
	if (IsValid(AIController) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	UBlackboardComponent* BB = AIController->GetBlackboardComponent();
	if (IsValid(BB) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (IsValid(Target) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	// 예외 상황이없을 시 점멸 실행
	ExecuteBlink(Monster, Target);
	ApplyAreaDamage(Monster);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_BossBlinkAbility::ExecuteBlink(AEDMonsterBase* Monster, AActor* Target)
{
	Monster->SetActorLocation(BlinkDestination, false, nullptr, ETeleportType::TeleportPhysics);
	
	// 점멸 후 타겟 방향으로 회전
	if (IsValid(Target))
	{
		FVector LookDir = (Target->GetActorLocation() - BlinkDestination).GetSafeNormal();
		Monster->SetActorRotation(LookDir.Rotation());
	}
	// 착지 이펙트
	if (IsValid(BlinkArrivalEffect))
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(Monster, BlinkArrivalEffect, BlinkDestination);
	// 착지 사운드
	if (IsValid(BlinkArrivalSound))
		UGameplayStatics::PlaySoundAtLocation(Monster, BlinkArrivalSound, BlinkDestination);
}

void UGA_BossBlinkAbility::ApplyAreaDamage(AEDMonsterBase* Monster)
{
	if (IsValid(DamageEffectClass) == false)
		return;
	
	UAbilitySystemComponent* MonsterASC = Monster->GetAbilitySystemComponent();
	if (IsValid(MonsterASC) == false)
		return;
	
	TArray<AActor*> OverlapActor;
	TArray<AActor*> ActorToIgnore = { Monster };
	UKismetSystemLibrary::SphereOverlapActors(
		Monster, BlinkDestination, DamageRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) },
		nullptr, ActorToIgnore, OverlapActor);
	
	const float Atk = IsValid(Monster->GetDataAsset()) ? Monster->GetDataAsset()->GetStat().Atk : 0.f;
	// 오버랩 된 Actor 순회하면서 범위 데미지 적용
	for (AActor* HitActor : OverlapActor)
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
		FGameplayEffectSpecHandle SpecHandle = MonsterASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
		if (SpecHandle.IsValid() == false)
			continue;
		
		SpecHandle.Data->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_Damage, Atk);
		MonsterASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}
