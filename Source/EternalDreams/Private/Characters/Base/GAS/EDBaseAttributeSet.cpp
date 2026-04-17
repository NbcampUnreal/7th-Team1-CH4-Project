// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Base/GAS/EDBaseAttributeSet.h"

#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UEDBaseAttributeSet::UEDBaseAttributeSet()
{
	InitMaxHealth(MaxHealthFloat);
	InitMaxDefensive(MaxDefensiveFloat);
	InitMaxWalkSpeed(MaxWalkSpeedFloat);
	
	InitHealth(GetMaxHealth());
	InitDefensive(GetMaxDefensive());
	InitWalkSpeed(GetMaxWalkSpeed());
}

void UEDBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UEDBaseAttributeSet,Health);
	DOREPLIFETIME(UEDBaseAttributeSet,MaxHealth);
	
	DOREPLIFETIME(UEDBaseAttributeSet,Defensive);
	DOREPLIFETIME(UEDBaseAttributeSet,MaxDefensive);	
	
	DOREPLIFETIME(UEDBaseAttributeSet,WalkSpeed);
	DOREPLIFETIME(UEDBaseAttributeSet,MaxWalkSpeed);	
}

void UEDBaseAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDBaseAttributeSet,Health, OldHealth);
}

void UEDBaseAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDBaseAttributeSet,MaxHealth, OldMaxHealth);
}

void UEDBaseAttributeSet::OnRep_Defensive(const FGameplayAttributeData& OldDefensive)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDBaseAttributeSet,Defensive, OldDefensive);
}

void UEDBaseAttributeSet::OnRep_MaxDefensive(const FGameplayAttributeData& OldMaxDefensive)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDBaseAttributeSet,MaxDefensive, OldMaxDefensive);
}

void UEDBaseAttributeSet::OnRep_WalkSpeed(const FGameplayAttributeData& OldWalkSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDBaseAttributeSet,WalkSpeed, OldWalkSpeed);
}

void UEDBaseAttributeSet::OnRep_MaxWalkSpeed(const FGameplayAttributeData& OldMaxWalkSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDBaseAttributeSet,WalkSpeed, OldMaxWalkSpeed);
}

void UEDBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	//Clamp
	if (Attribute==GetHealthAttribute())
	{
		NewValue=FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	if (Attribute==GetDefensiveAttribute())
	{
		NewValue=FMath::Clamp(NewValue, 0.0f, GetMaxDefensive());
	}
	if (Attribute==GetWalkSpeedAttribute())
	{
		NewValue=FMath::Clamp(NewValue, 0.0f, MaxAttributeValue);
	}
}

void UEDBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	//Clamp
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Health가 변경되었을 때
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		// HP 0 감지 → Target 타입별 사망 진입점 호출
		if (GetHealth() <= 0.0f)
		{
			AActor* TargetActor = Data.Target.GetAvatarActor();

			// Killer 추출: Instigator Pawn → Controller
			AController* Killer = nullptr;
			if (AActor* InstigatorActor = Data.EffectSpec.GetContext().GetInstigator())
			{
				if (APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
				{
					Killer = InstigatorPawn->GetController();
				}
				else
				{
					Killer = Cast<AController>(InstigatorActor);
				}
			}

			if (AEDPlayerCharacter* Player = Cast<AEDPlayerCharacter>(TargetActor))
			{
				Player->HandleDeath(Killer);
			}
			else if (AEDMonsterBase* Monster = Cast<AEDMonsterBase>(TargetActor))
			{
				Monster->HandleDeath();
			}
		}
	}
	if (Data.EvaluatedData.Attribute == GetDefensiveAttribute())
	{
		// Defensive가 변경되었을 때
		SetDefensive(FMath::Clamp(GetDefensive(), 0.0f, GetMaxDefensive()));
	}
	if (Data.EvaluatedData.Attribute == GetWalkSpeedAttribute())
	{
		// WalkSpeed가 변경되었을 때
		SetWalkSpeed(FMath::Clamp(GetWalkSpeed(), 0.0f, MaxAttributeValue));
	}
}








