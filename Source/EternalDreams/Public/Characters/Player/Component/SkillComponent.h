// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"


class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USkillComponent();
	
	virtual void BeginPlay() override;

protected:
	//Skill Tags
	UPROPERTY(EditAnyWhere)
	FGameplayTag BasicAttackTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag QSkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag ESkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag SpaceSkillTag=FGameplayTag::EmptyTag;
	
	//Caching
protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent=nullptr;

	
#pragma region Getter/Setter
public:
	UFUNCTION()
	FORCEINLINE void SetBasicAttackTag(FGameplayTag& Tag) {BasicAttackTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetBasicAttackTag() {return BasicAttackTag;};

	UFUNCTION()
	FORCEINLINE void SetQSkillTag(FGameplayTag& Tag) {QSkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetQSkillTag() {return QSkillTag;}
	
	UFUNCTION()
	FORCEINLINE void SetESkillTag(FGameplayTag& Tag) {ESkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSetESkillTag() {return ESkillTag;};
	
	UFUNCTION()
	FORCEINLINE void SetSpaceSkillTag(FGameplayTag& Tag) {SpaceSkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSpaceSkillTag() {return SpaceSkillTag;};
#pragma endregion

	//Activate Skill
public:
	UFUNCTION()
	void ActivateBasicAttack();
	UFUNCTION()
	void ActivateQSkill();
	UFUNCTION()
	void ActivateESkill();
	UFUNCTION()
	void ActivateSpaceSkill();
	

};
