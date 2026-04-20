// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/SkillComponent.h"

#include "AbilitySystemComponent.h"


void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent=GetOwner()->FindComponentByClass<UAbilitySystemComponent>();

}

void USkillComponent::SetQSkillCoolTimeTag(const FGameplayTag Tag)
{
	if (QSkillCoolTimeTag!=FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(QSkillCoolTimeTag).Remove(QSkillCoolTimeHandle);
	}
	QSkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(Tag).AddUObject(this,&USkillComponent::QSkillCoolTime);
	QSkillCoolTimeTag=Tag;
}

void USkillComponent::SetESkillCoolTimeTag(const FGameplayTag Tag)
{
	if (ESkillCoolTimeTag!=FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(ESkillCoolTimeTag).Remove(ESkillCoolTimeHandle);
	}
	ESkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(Tag).AddUObject(this,&USkillComponent::ESkillCoolTime);
	ESkillCoolTimeTag=Tag;
}


void USkillComponent::SetSpaceSkillCoolTimeTag(const FGameplayTag Tag)
{
	if (SpaceSkillCoolTimeTag!=FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(SpaceSkillCoolTimeTag).Remove(SpaceSkillCoolTimeHandle);
	}
	SpaceSkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(Tag).AddUObject(this,&USkillComponent::SpaceSkillCoolTime);
	SpaceSkillCoolTimeTag=Tag;
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
		AbilityTagContainer.AddTag(Tag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
}

float USkillComponent::CalculateCoolTime(FGameplayTag& Tag)
{
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	TArray<float> Times = AbilitySystemComponent->GetActiveEffectsTimeRemaining(Query);
	if (!Times.IsEmpty())
	{
		return Times[0];
	}
	return 0.f;
}

float USkillComponent::CalculateMaxCoolTime(FGameplayTag& Tag)
{
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	TArray<float> Times = AbilitySystemComponent->GetActiveEffectsDuration(Query);
	if (!Times.IsEmpty())
	{
		return Times[0];
	}
	return 0.f;
}

void USkillComponent::QSkillCoolTime(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount!=0)
	{
		OnQSkillCoolTime.Broadcast(CalculateCoolTime(Tag),CalculateMaxCoolTime(Tag));
	}
	else
	{
		OnQSkillCoolTime.Broadcast(0.f,0.f);
	}
}

void USkillComponent::ESkillCoolTime(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount!=0)
	{
		OnESkillCoolTime.Broadcast(CalculateCoolTime(Tag),CalculateMaxCoolTime(Tag));
	}
	else
	{
		OnESkillCoolTime.Broadcast(0.f,0.f);
	}
}

void USkillComponent::SpaceSkillCoolTime(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount!=0)
	{
		OnSpaceSkillCoolTime.Broadcast(CalculateCoolTime(Tag),CalculateMaxCoolTime(Tag));
	}
	else
	{
		OnSpaceSkillCoolTime.Broadcast(0.f,0.f);
	}
}
