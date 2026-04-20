// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BossBlinkAbility.generated.h"

class UGameplayEffect;
class UNiagaraSystem;
class AEDMonsterBase;

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UGA_BossBlinkAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_BossBlinkAbility();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		bool bReplicateEndAbility, 
		bool bWasCancelled) override;
	// 점멸 목적지까지 오프셋(타겟 뒤쪽)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blink|Data")
	float BlinkOffset = 150.f;
	// 인디케이터 표시 후 점멸 실행 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blink|Data")
	float BlinkDelay = 1.5;
	// 범위 데미지 반경
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blink|Data")
	float DamageRadius = 300.f;
	// 범위 데미지 GE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blink|Data")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	// 점멸 목적지 인디케이터 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blink|Effect")
	TObjectPtr<UNiagaraSystem> IndicatorEffect;
	// 점멸 착지 시 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blink|Effect")
	TObjectPtr<UNiagaraSystem> BlinkArrivalEffect;

private:
	UFUNCTION()
	void OnBlinkDelayFinished();
	
	void ExecuteBlink(AEDMonsterBase* Monster, AActor* Target);
	void ApplyAreaDamage(AEDMonsterBase* Monster);
	
	FVector BlinkDestination;
};
