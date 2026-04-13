// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/SkillComponent.h"

#include "AbilitySystemComponent.h"
#include "Data/GameplayTag/EDGameplayTags.h"


// Sets default values for this component's properties
USkillComponent::USkillComponent()
{
	

}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent=GetOwner()->FindComponentByClass<UAbilitySystemComponent>();
}

void USkillComponent::ActivateBasicAttack()
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer AbilityTagContainer;
		const FEDGameplayTags& Tags = FEDGameplayTags::Get();
		AbilityTagContainer.AddTag(BasicAttackTag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
}

void USkillComponent::ActivateQSkill()
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer AbilityTagContainer;
		const FEDGameplayTags& Tags = FEDGameplayTags::Get();
		AbilityTagContainer.AddTag(QSkillTag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
}

void USkillComponent::ActivateESkill()
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer AbilityTagContainer;
		const FEDGameplayTags& Tags = FEDGameplayTags::Get();
		AbilityTagContainer.AddTag(ESkillTag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
}

void USkillComponent::ActivateSpaceSkill()
{
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer AbilityTagContainer;
		const FEDGameplayTags& Tags = FEDGameplayTags::Get();
		AbilityTagContainer.AddTag(SpaceSkillTag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
}