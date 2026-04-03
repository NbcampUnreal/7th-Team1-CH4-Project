// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include"GameplayEffect.h"
#include"GameplayEffectExtension.h"

UEDPlayerAttributeSet::UEDPlayerAttributeSet()
{
	
}

void UEDPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}

void UEDPlayerAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, Mana, OldMana);
}

void UEDPlayerAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, MaxMana, OldMaxMana);
}

void UEDPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	//Clamp
	if (Attribute==GetManaAttribute())
	{
		NewValue=FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
}

void UEDPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute != GetManaAttribute())
	{
		// Mana가 변경되었을 때
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
}



