// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Delegates/IDelegateInstance.h"
#include "SkillComponent.generated.h"



class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillCoolTime,float,SkillCoolTime,float,MaxSkillCoolTime);

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
	
		
	//Delegates
	public:
	UPROPERTY(BlueprintAssignable)
	FOnSkillCoolTime OnQSkillCoolTime;
	UPROPERTY(BlueprintAssignable)
	FOnSkillCoolTime OnESkillCoolTime;
	UPROPERTY(BlueprintAssignable)
	FOnSkillCoolTime OnSpaceSkillCoolTime;
	
	
	
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
	void SetQSkillCoolTimeTag(const FGameplayTag Tag); 
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetQSkillCoolTimeTag() {return QSkillCoolTimeTag;}
	
	UFUNCTION(BlueprintCallable)
	void SetESkillTag(const FGameplayTag Tag) {ESkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetESkillTag() {return ESkillTag;};
	
	UFUNCTION(BlueprintCallable)
	void SetESkillCoolTimeTag(const FGameplayTag Tag) ;
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetESkillCoolTimeTag() {return ESkillCoolTimeTag;};
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetSpaceSkillTag(const FGameplayTag Tag) {SpaceSkillTag=Tag;};
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSpaceSkillTag() {return SpaceSkillTag;};
	
	UFUNCTION(BlueprintCallable)
	void SetSpaceSkillCoolTimeTag(const FGameplayTag Tag);
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSpaceSkillCoolTimeTag() {return SpaceSkillCoolTimeTag;};
	
#pragma endregion

	//Handles
protected:
	FDelegateHandle QSkillCoolTimeHandle;
	FDelegateHandle ESkillCoolTimeHandle;
	FDelegateHandle SpaceSkillCoolTimeHandle;
	
	
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
	
	
	//CallBacks
public:
	UFUNCTION()
	void QSkillCoolTime(FGameplayTag Tag, int32 NewCount);
	UFUNCTION()
	void ESkillCoolTime(FGameplayTag Tag, int32 NewCount);
	UFUNCTION()
	void SpaceSkillCoolTime(FGameplayTag Tag, int32 NewCount);

	
	
	//Activate Skill By Tag
protected:
	void ActivateTag(FGameplayTag& Tag);
	
	float CalculateCoolTime(FGameplayTag& Tag);
	float CalculateMaxCoolTime(FGameplayTag& Tag);

	
	
};
