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
	
	virtual void BeginPlay() override;

protected:
	//Skill Tags
	UPROPERTY(EditAnyWhere)
	FGameplayTag BasicAttackTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag QSkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag QSkillCoolTimeTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag ESkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag ESkillCoolTimeTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag SpaceSkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere)
	FGameplayTag SpaceSkillCoolTimeTag=FGameplayTag::EmptyTag;
	
	//Caching
protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent=nullptr;

	
#pragma region Getter/Setter
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetBasicAttackTag(const FGameplayTag Tag) {BasicAttackTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetBasicAttackTag() {return BasicAttackTag;};

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetQSkillTag(const FGameplayTag Tag) {QSkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetQSkillTag() {return QSkillTag;}
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetQSkillCoolTimeTag(const FGameplayTag Tag) {QSkillCoolTimeTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetQSkillCoolTimeTag() {return QSkillCoolTimeTag;}
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetESkillTag(const FGameplayTag Tag) {ESkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSetESkillTag() {return ESkillTag;};
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetESkillCoolTimeTag(const FGameplayTag Tag) {ESkillCoolTimeTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSetESkillCoolTimeTag() {return ESkillCoolTimeTag;};
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetSpaceSkillTag(const FGameplayTag Tag) {SpaceSkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSpaceSkillTag() {return SpaceSkillTag;};
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetSpaceSkillCoolTimeTag(const FGameplayTag Tag) {SpaceSkillCoolTimeTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSpaceSkillCoolTimeTag() {return SpaceSkillCoolTimeTag;};
	
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
	
//Activate Skill By Tag
protected:
	void ActivateTag(FGameplayTag& Tag);
	
};
