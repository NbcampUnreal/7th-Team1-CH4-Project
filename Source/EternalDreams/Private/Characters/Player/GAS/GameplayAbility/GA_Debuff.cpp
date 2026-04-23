// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/GAS/GameplayAbility/GA_Debuff.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Core/EDGameDataSubsystem.h"
#include "Data/EDPlayerAnimDataAsset.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"

UGA_Debuff::UGA_Debuff()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGA_Debuff::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_Debuff::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	//Commit
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		//Commit Failed
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	//Get ASC
	UAbilitySystemComponent* PlayerASC = GetAbilitySystemComponentFromActorInfo();
	if (!PlayerASC)
	{
		return;
	}
	//Get AttributeSet
	const UEDPlayerAttributeSet* PlayerAttributeSet = Cast<UEDPlayerAttributeSet>(PlayerASC->GetAttributeSet(UEDPlayerAttributeSet::StaticClass()));
	if (!IsValid(PlayerAttributeSet))
	{
		return;
	}
	
	//Get AnimMontage

	UEDGameDataSubsystem* EDGameDataSubsystem = UEDGameDataSubsystem::Get(GetWorld());
	if (IsValid(EDGameDataSubsystem))
	{
		UEDPlayerAnimDataAsset* PlayerAnimData = EDGameDataSubsystem->GetData<UEDPlayerAnimDataAsset>(
			FPrimaryAssetId(
				*UEnum::GetDisplayValueAsText(EPlayerDataType::PlayerAnimData).ToString(),
				*UEnum::GetDisplayValueAsText(MontageName).ToString()
			));
		UE_LOG(LogTemp,Warning,TEXT("%s %s"),*UEnum::GetDisplayValueAsText(EPlayerDataType::PlayerAnimData).ToString(),*UEnum::GetDisplayValueAsText(MontageName).ToString());
		
		if (IsValid(PlayerAnimData))
		{
			UE_LOG(LogTemp,Warning,TEXT("PlayerAnimData Exist"));
			
			
			// 1. 경로 자체가 비어있는지 확인
			UE_LOG(LogTemp, Log, TEXT("Path: %s"), *PlayerAnimData->AnimMontage.ToString());
    
			// 2. 이미 로드된 상태인지(IsPending) 확인
			bool bIsLoaded = PlayerAnimData->AnimMontage.IsPending() == false;
			UE_LOG(LogTemp, Log, TEXT("Is Loaded: %s"), bIsLoaded ? TEXT("True") : TEXT("False"));

			PlayerAnimMontage = PlayerAnimData->AnimMontage.LoadSynchronous();
			if (IsValid(PlayerAnimMontage))
			{
				UE_LOG(LogTemp,Warning,TEXT("PlayerAnimMontage Valid"));
			}
		}
		else
		{
			{
				UE_LOG(LogTemp,Warning,TEXT("PlayerAnimData Not"));
			}
		}
		
	}
	
	//Play Montage Task(비동기)
	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			PlayerAnimMontage,
			1
		);
	if (!PlayMontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// Task 활성화
	PlayMontageTask->ReadyForActivation();
	
	
	const FGameplayTagContainer& AssetTags=GetAssetTags();
	FGameplayTag DebuffTag=AssetTags.GetByIndex(0);
	UAbilityTask_WaitGameplayTagRemoved* WaitTagRemovedTask = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
	this, 
				DebuffTag, 
nullptr, 
false 
		);
	
	if (WaitTagRemovedTask)
	{
		WaitTagRemovedTask->Removed.AddDynamic(this, &UGA_Debuff::OnDebuffTagRemoved);
		WaitTagRemovedTask->ReadyForActivation();
	}
	
}

void UGA_Debuff::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Debuff::OnDebuffTagRemoved()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

