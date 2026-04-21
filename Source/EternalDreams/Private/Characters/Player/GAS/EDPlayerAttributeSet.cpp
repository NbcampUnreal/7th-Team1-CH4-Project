// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include"GameplayEffect.h"
#include"GameplayEffectExtension.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Engine/Engine.h"

UEDPlayerAttributeSet::UEDPlayerAttributeSet()
{
	//초기화
	InitStrength(10.0f);
	InitDexterity(10.0f);
	InitIntelligence(10.0f);
	InitAttackSpeed(1.f);
	
	//KSH --- 금지구역시간 초기화 (필요시 수치 변경)
	InitSurvivalTime(30.0f);
	InitMaxSurvivalTime(30.0f);
}

void UEDPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UEDPlayerAttributeSet,Strength);
	DOREPLIFETIME(UEDPlayerAttributeSet,Dexterity);
	DOREPLIFETIME(UEDPlayerAttributeSet,Intelligence);
	DOREPLIFETIME(UEDPlayerAttributeSet,AttackSpeed);
	
	// KSH --- 금지구역 시간 복제 
	DOREPLIFETIME_CONDITION_NOTIFY(UEDPlayerAttributeSet, SurvivalTime, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEDPlayerAttributeSet, MaxSurvivalTime, COND_None, REPNOTIFY_Always);
}


void UEDPlayerAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, Strength, OldStrength);
}

void UEDPlayerAttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldDexterity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, Dexterity, OldDexterity);
}

void UEDPlayerAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, Intelligence, OldIntelligence);
}

void UEDPlayerAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, AttackSpeed, OldAttackSpeed);
}

// KSH --- 금지구역 콜백 함수 구현
void UEDPlayerAttributeSet::OnRep_SurvivalTime(const FGameplayAttributeData& OldSurvivalTime)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, SurvivalTime, OldSurvivalTime);
}

void UEDPlayerAttributeSet::OnRep_MaxSurvivalTime(const FGameplayAttributeData& OldMaxSurvivalTime)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, MaxSurvivalTime, OldMaxSurvivalTime);
}


void UEDPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	//Clamp
	if (Attribute == GetStrengthAttribute()||
		Attribute==GetDexterityAttribute()||
		Attribute==GetIntelligenceAttribute()||
		Attribute==GetAttackSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, MaxAttributeValue);
	}
	
	// KSH --- 금지구역 생존시간 Clamp 추가
	else if (Attribute == GetSurvivalTimeAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxSurvivalTime());
	}
}

void UEDPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	//변화량
	float DeltaValue = Data.EvaluatedData.Magnitude;
	
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	
	
	if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
	{
		// Strength가 변경되었을 때
		SetStrength(FMath::Clamp(GetStrength(), 0.0f, MaxAttributeValue));
		
	}
	if (Data.EvaluatedData.Attribute == GetDexterityAttribute())
	{
		// Dexterity가 변경되었을 때
		SetDexterity(FMath::Clamp(GetDexterity(), 0.0f, MaxAttributeValue));
		
	}
	if (Data.EvaluatedData.Attribute == GetIntelligenceAttribute())
	{
		// Intelligence가 변경되었을 때
		SetIntelligence(FMath::Clamp(GetIntelligence(), 0.0f, MaxAttributeValue));


	}
	if (Data.EvaluatedData.Attribute == GetAttackSpeedAttribute())
	{
		// AttackSpeed가 변경되었을 때
		SetAttackSpeed(FMath::Clamp(GetAttackSpeed(), 0.0f, MaxAttributeValue));
	}
	
	// KSH --- 금지구역 생존시간 감소 및 디버그 메시지 처리
	if (Data.EvaluatedData.Attribute == GetSurvivalTimeAttribute())
	{
		// 1. Clamp 되기 전의 실제 계산된 값 (예: 원래 1초였는데 1 깎이면 0.0, 원래 0초였는데 1 깎이면 -1.0)
		float UnclampedTime = GetSurvivalTime();

		// 2. 값을 안전하게 0 ~ 최대치로 Clamp
		SetSurvivalTime(FMath::Clamp(UnclampedTime, 0.0f, GetMaxSurvivalTime()));

		float CurrentTime = GetSurvivalTime();

		if (CurrentTime > 0.0f)
		{
			// 1초마다 생존 시간 갱신
			if (GEngine)
			{
				FString Msg = FString::Printf(TEXT("금지구역 생존 시간: %.0f초 남았습니다!"), CurrentTime);
				GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Yellow, Msg);
			}
		}
		else if (CurrentTime <= 0.0f)
		{
			// 3. 도배 방지: 방금 딱 0 이하가 된 순간에만 실행
			if (UnclampedTime > -0.99f)
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("생존 시간이 0이 되었습니다! 플레이어 사망!"));
				}

				// 금지구역 사망 — 환경 데미지이므로 Killer 없음
				if (AEDPlayerCharacter* Player = Cast<AEDPlayerCharacter>(Data.Target.GetAvatarActor()))
				{
					Player->HandleDeath(nullptr);
				}
			}
		}
	}
	
	
}
