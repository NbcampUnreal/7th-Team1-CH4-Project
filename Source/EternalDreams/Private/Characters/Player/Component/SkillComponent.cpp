// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/SkillComponent.h"

#include "AbilitySystemComponent.h"
#include "Data/GameplayTag/EDGameplayTags.h"


void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent=GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
}

void USkillComponent::ActivateBasicAttack()
{
	ActivateTag(BasicAttackTag);
}

void USkillComponent::ActivateQSkill()
{
	ActivateTag(QSkillTag);
}

void USkillComponent::ActivateESkill()
{
	ActivateTag(ESkillTag);
}

void USkillComponent::ActivateSpaceSkill()
{
	ActivateTag(SpaceSkillTag);
}

void USkillComponent::ActivateTag(FGameplayTag& Tag)
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer AbilityTagContainer;
		const FEDGameplayTags& Tags = FEDGameplayTags::Get();
		AbilityTagContainer.AddTag(Tag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
}
