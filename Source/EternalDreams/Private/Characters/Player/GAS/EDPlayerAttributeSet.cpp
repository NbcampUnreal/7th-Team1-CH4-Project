// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include"GameplayEffect.h"
#include"GameplayEffectExtension.h"
#include "Engine/Engine.h"

UEDPlayerAttributeSet::UEDPlayerAttributeSet()
{
	//KSH --- 금지구역시간 초기화 (필요시 수치 변경)
	InitSurvivalTime(30.0f);
	InitMaxSurvivalTime(30.0f);
}

void UEDPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// KSH --- 금지구역 시간 복제 
	DOREPLIFETIME_CONDITION_NOTIFY(UEDPlayerAttributeSet, SurvivalTime, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UEDPlayerAttributeSet, MaxSurvivalTime, COND_None, REPNOTIFY_Always);
}

void UEDPlayerAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, Mana, OldMana);
}

void UEDPlayerAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UEDPlayerAttributeSet, MaxMana, OldMaxMana);
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
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
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

	if (Data.EvaluatedData.Attribute != GetManaAttribute())
	{
		// Mana가 변경되었을 때
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
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
               
				// TODO: 추후 Character->Die() 같은 사망 함수를 한 번만 호출
			}
		}
	}
	
	
}
