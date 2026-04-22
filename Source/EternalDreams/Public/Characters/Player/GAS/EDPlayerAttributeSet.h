// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include"AbilitySystemComponent.h"
#include "EDPlayerAttributeSet.generated.h"

// Attribute Accessors 매크로
#define ATTRIBUTE_ACCESSORS(ClassName,PropertyName)\
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName,PropertyName)\
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)\
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)\
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)



/*
 * 공용 Attribute를 제외한 플레이어 Attribute를 관리하는 AttributeSet
 */
UCLASS()
class ETERNALDREAMS_API UEDPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UEDPlayerAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//속성
public:

	
	//힘
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UEDPlayerAttributeSet, Strength)
	//민첩
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Dexterity)
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UEDPlayerAttributeSet, Dexterity)
	//지능
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Intelligence)
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UEDPlayerAttributeSet, Intelligence)
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackSpeed)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UEDPlayerAttributeSet, AttackSpeed)
	
	//kSH --- 금지구역 시간 ---
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_SurvivalTime)
	FGameplayAttributeData SurvivalTime;
	ATTRIBUTE_ACCESSORS(UEDPlayerAttributeSet, SurvivalTime)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxSurvivalTime)
	FGameplayAttributeData MaxSurvivalTime;
	ATTRIBUTE_ACCESSORS(UEDPlayerAttributeSet, MaxSurvivalTime)
	
	//콜백 함수
public:
	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength);
	UFUNCTION()
	virtual void OnRep_Dexterity(const FGameplayAttributeData& OldDexterity);
	UFUNCTION()
	virtual void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence);
	UFUNCTION()
	virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);


	
	//kSH --- 금지구역 콜백 ---
	UFUNCTION()
	virtual void OnRep_SurvivalTime(const FGameplayAttributeData& OldSurvivalTime);
	UFUNCTION()
	virtual void OnRep_MaxSurvivalTime(const FGameplayAttributeData& OldMaxSurvivalTime);
	
public:
	// Attribute 변경 전 호출 (Clamping)
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Attribute 변경 후 호출
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	
protected:
	UPROPERTY()
	float MaxAttributeValue=9999.f;
};
