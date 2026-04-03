// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "EDBaseAttributeSet.generated.h"

// Attribute Accessors 매크로
#define ATTRIBUTE_ACCESSORS(ClassName,PropertyName)\
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName,PropertyName)\
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)\
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)\
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/*
 * [BaseAttributeSet]
 * 체력, 방어력, 이동속도의 현재와 최대치(혹은 감소되기 전 값) Attribute를 관리하는 캐릭터, 몬스터의 공통된 AttributeSet
 */
UCLASS()
class ETERNALDREAMS_API UEDBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
	
public:
	UEDBaseAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//값 초기 설정용 속성
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Attributes")
	float MaxHealthFloat=100.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Attributes")
	float MaxDefensiveFloat=0.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Attributes")
	float MaxWalkSpeedFloat=680.f;
	
	
	
	//속성
public:
	//현재 체력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UEDBaseAttributeSet, Health)
	
	//최대 체력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UEDBaseAttributeSet, MaxHealth)
	
	//현재 방어력
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Defensive)
	FGameplayAttributeData Defensive;
	ATTRIBUTE_ACCESSORS(UEDBaseAttributeSet, Defensive)
	
	//최대 방어력(감소되기 전 방어력)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxDefensive)
	FGameplayAttributeData MaxDefensive;
	ATTRIBUTE_ACCESSORS(UEDBaseAttributeSet, MaxDefensive)
	
	//현재 이동속도
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_WalkSpeed)
	FGameplayAttributeData WalkSpeed;
	ATTRIBUTE_ACCESSORS(UEDBaseAttributeSet, WalkSpeed)
	
	//최대 이동속도(감소되기 전 이동속도)
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxWalkSpeed)
	FGameplayAttributeData MaxWalkSpeed;
	ATTRIBUTE_ACCESSORS(UEDBaseAttributeSet, MaxWalkSpeed)
	
	
	
	//콜백 함수
public:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	
	UFUNCTION()
	virtual void OnRep_Defensive(const FGameplayAttributeData& OldDefensive);
	UFUNCTION()
	virtual void OnRep_MaxDefensive(const FGameplayAttributeData& OldMaxDefensive);
	
	UFUNCTION()
	virtual void OnRep_WalkSpeed(const FGameplayAttributeData& OldWalkSpeed);
	UFUNCTION()
	virtual void OnRep_MaxWalkSpeed(const FGameplayAttributeData& OldMaxWalkSpeed);

	
public:
	// Attribute 변경 전 호출 (Clamping)
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Attribute 변경 후 호출
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	
};
