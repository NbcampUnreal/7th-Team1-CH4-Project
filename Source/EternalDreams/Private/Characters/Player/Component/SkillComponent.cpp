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
	EnsureAbilitySystemComponent();

	OnRep_QSkillCoolTimeTag();
	OnRep_ESkillCoolTimeTag();
	OnRep_SpaceSkillCoolTimeTag();

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
	if (EnsureAbilitySystemComponent() && Tag.IsValid())
	{
		FGameplayTagContainer AbilityTagContainer;
		AbilityTagContainer.AddTag(Tag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
}

float USkillComponent::CalculateCoolTime(FGameplayTag& Tag)
{
	if (!EnsureAbilitySystemComponent() || !Tag.IsValid())
	{
		return 0.f;
	}

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
	if (!EnsureAbilitySystemComponent() || !Tag.IsValid())
	{
		return 0.f;
	}

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
	if (!EnsureAbilitySystemComponent())
	{
		return;
	}

	if (PastQSkillCoolTimeTag.IsValid() && QSkillCoolTimeHandle.IsValid())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(PastQSkillCoolTimeTag).Remove(QSkillCoolTimeHandle);
		QSkillCoolTimeHandle.Reset();
	}

	if (!QSkillCoolTimeTag.IsValid())
	{
		PastQSkillCoolTimeTag = FGameplayTag::EmptyTag;
		OnQSkillCoolTime.Broadcast(0.f, 0.f);
		return;
	}

	QSkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(QSkillCoolTimeTag).AddUObject(this,&USkillComponent::QSkillCoolTime);
	PastQSkillCoolTimeTag=QSkillCoolTimeTag;
}

void USkillComponent::OnRep_ESkillCoolTimeTag()
{
	if (!EnsureAbilitySystemComponent())
	{
		return;
	}

	if (PastESkillCoolTimeTag.IsValid() && ESkillCoolTimeHandle.IsValid())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(PastESkillCoolTimeTag).Remove(ESkillCoolTimeHandle);
		ESkillCoolTimeHandle.Reset();
	}

	if (!ESkillCoolTimeTag.IsValid())
	{
		PastESkillCoolTimeTag = FGameplayTag::EmptyTag;
		OnESkillCoolTime.Broadcast(0.f, 0.f);
		return;
	}

	ESkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(ESkillCoolTimeTag).AddUObject(this,&USkillComponent::ESkillCoolTime);
	PastESkillCoolTimeTag=ESkillCoolTimeTag;
}

void USkillComponent::OnRep_SpaceSkillCoolTimeTag()
{
	if (!EnsureAbilitySystemComponent())
	{
		return;
	}

	if (PastSpaceSkillCoolTimeTag.IsValid() && SpaceSkillCoolTimeHandle.IsValid())
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(PastSpaceSkillCoolTimeTag).Remove(SpaceSkillCoolTimeHandle);
		SpaceSkillCoolTimeHandle.Reset();
	}

	if (!SpaceSkillCoolTimeTag.IsValid())
	{
		PastSpaceSkillCoolTimeTag = FGameplayTag::EmptyTag;
		OnSpaceSkillCoolTime.Broadcast(0.f, 0.f);
		return;
	}

	SpaceSkillCoolTimeHandle=AbilitySystemComponent->RegisterGameplayTagEvent(SpaceSkillCoolTimeTag).AddUObject(this,&USkillComponent::SpaceSkillCoolTime);
	PastSpaceSkillCoolTimeTag=SpaceSkillCoolTimeTag;
}

bool USkillComponent::EnsureAbilitySystemComponent()
{
	if (IsValid(AbilitySystemComponent))
	{
		return true;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return false;
	}

	AbilitySystemComponent = Owner->FindComponentByClass<UAbilitySystemComponent>();
	return IsValid(AbilitySystemComponent);
}
