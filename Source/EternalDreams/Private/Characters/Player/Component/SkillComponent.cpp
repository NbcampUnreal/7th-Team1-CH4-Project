// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/SkillComponent.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"


USkillComponent::USkillComponent()
{
	SetIsReplicatedByDefault(true);
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent=GetOwner()->FindComponentByClass<UAbilitySystemComponent>();

}

void USkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(USkillComponent, BasicAttackTag);
	DOREPLIFETIME(USkillComponent, QSkillTag);
	DOREPLIFETIME(USkillComponent, QSkillCoolTimeTag);
	DOREPLIFETIME(USkillComponent, ESkillTag);
	DOREPLIFETIME(USkillComponent, ESkillCoolTimeTag);
	DOREPLIFETIME(USkillComponent, SpaceSkillTag);
	DOREPLIFETIME(USkillComponent, SpaceSkillCoolTimeTag);
}

void USkillComponent::SetBasicAttackTag_Implementation(const FGameplayTag Tag)
{
	BasicAttackTag=Tag;
}

void USkillComponent::SetQSkillTag_Implementation(const FGameplayTag Tag)
{
	QSkillTag=Tag;
}

void USkillComponent::SetQSkillCoolTimeTag_Implementation(const FGameplayTag Tag)
{
	QSkillCoolTimeTag=Tag;
}

void USkillComponent::SetESkillTag_Implementation(const FGameplayTag Tag)
{
	ESkillTag=Tag;
}

void USkillComponent::SetESkillCoolTimeTag_Implementation(const FGameplayTag Tag)
{
	ESkillCoolTimeTag=Tag;
}


void USkillComponent::SetSpaceSkillTag_Implementation(const FGameplayTag Tag)
{
	SpaceSkillTag=Tag;
}

void USkillComponent::SetSpaceSkillCoolTimeTag_Implementation(const FGameplayTag Tag)
{
	SpaceSkillCoolTimeTag=Tag;
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

void USkillComponent::OnRep_QSkillCoolTimeTag()
{
	if (PastQSkillCoolTimeTag!=FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(PastQSkillCoolTimeTag).Remove(QSkillCoolTimeHandle);
	}
	QSkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(QSkillCoolTimeTag).AddUObject(this,&USkillComponent::QSkillCoolTime);
	PastQSkillCoolTimeTag=QSkillCoolTimeTag;
}

void USkillComponent::OnRep_ESkillCoolTimeTag()
{
	if (PastESkillCoolTimeTag!=FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(PastESkillCoolTimeTag).Remove(ESkillCoolTimeHandle);
	}
	ESkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(ESkillCoolTimeTag).AddUObject(this,&USkillComponent::ESkillCoolTime);
	PastESkillCoolTimeTag=ESkillCoolTimeTag;
}

void USkillComponent::OnRep_SpaceSkillCoolTimeTag()
{
	if (PastSpaceSkillCoolTimeTag!=FGameplayTag::EmptyTag)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(PastSpaceSkillCoolTimeTag).Remove(SpaceSkillCoolTimeHandle);
	}
	SpaceSkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(SpaceSkillCoolTimeTag).AddUObject(this,&USkillComponent::SpaceSkillCoolTime);
	PastSpaceSkillCoolTimeTag=SpaceSkillCoolTimeTag;
}
