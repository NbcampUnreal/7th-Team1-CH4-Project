// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Delegates/IDelegateInstance.h"
#include "SkillComponent.generated.h"



class UAbilitySystemComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillSet,FGameplayTag,SkillTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillCoolTime,float,SkillCoolTime,float,MaxSkillCoolTime);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

protected:
	//Skill Tags
	UPROPERTY(EditAnyWhere,Replicated)
	FGameplayTag BasicAttackTag=FGameplayTag::EmptyTag;
	
	UPROPERTY(EditAnyWhere,ReplicatedUsing=OnRep_QSkillTag)
	FGameplayTag QSkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere,ReplicatedUsing=OnRep_QSkillCoolTimeTag)
	FGameplayTag QSkillCoolTimeTag=FGameplayTag::EmptyTag;
	UPROPERTY()
    FGameplayTag PastQSkillCoolTimeTag=FGameplayTag::EmptyTag;
	
	UPROPERTY(EditAnyWhere,ReplicatedUsing=OnRep_ESkillTag)
	FGameplayTag ESkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere,ReplicatedUsing=OnRep_ESkillCoolTimeTag)
	FGameplayTag ESkillCoolTimeTag=FGameplayTag::EmptyTag;
	UPROPERTY()
	FGameplayTag PastESkillCoolTimeTag=FGameplayTag::EmptyTag;
	
	UPROPERTY(EditAnyWhere,ReplicatedUsing=OnRep_SpaceSkillTag)
	FGameplayTag SpaceSkillTag=FGameplayTag::EmptyTag;
	UPROPERTY(EditAnyWhere,ReplicatedUsing=OnRep_SpaceSkillCoolTimeTag)
	FGameplayTag SpaceSkillCoolTimeTag=FGameplayTag::EmptyTag;
	UPROPERTY()
	FGameplayTag PastSpaceSkillCoolTimeTag=FGameplayTag::EmptyTag;
	
	
	//Delegates
	public:
	UPROPERTY(BlueprintAssignable)
	FOnSkillCoolTime OnQSkillCoolTime;
	UPROPERTY(BlueprintAssignable)
	FOnSkillCoolTime OnESkillCoolTime;
	UPROPERTY(BlueprintAssignable)
	FOnSkillCoolTime OnSpaceSkillCoolTime;
	
	UPROPERTY(BlueprintAssignable)
	FOnSkillSet OnQSkillSet;
	UPROPERTY(BlueprintAssignable)
	FOnSkillSet OnESkillSet;
	UPROPERTY(BlueprintAssignable)
	FOnSkillSet OnSpaceSkillSet;
	
	
	//Caching
protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent=nullptr;

	
#pragma region Getter/Setter
public:
	UFUNCTION(BlueprintCallable,Server,Reliable)
	void SetBasicAttackTag(const FGameplayTag Tag);
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetBasicAttackTag() {return BasicAttackTag;};

	UFUNCTION(BlueprintCallable,Server,Reliable)
	FORCEINLINE void SetQSkillTag(const FGameplayTag Tag);
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetQSkillTag() {return QSkillTag;}
	
	UFUNCTION(BlueprintCallable,Server,Reliable)
	void SetQSkillCoolTimeTag(const FGameplayTag Tag); 
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetQSkillCoolTimeTag() {return QSkillCoolTimeTag;}
	
	UFUNCTION(BlueprintCallable,Server,Reliable)
	void SetESkillTag(const FGameplayTag Tag);
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetESkillTag() {return ESkillTag;};
	
	UFUNCTION(BlueprintCallable,Server,Reliable)
	void SetESkillCoolTimeTag(const FGameplayTag Tag) ;
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetESkillCoolTimeTag() {return ESkillCoolTimeTag;};
	
	UFUNCTION(BlueprintCallable,Server,Reliable)
	void SetSpaceSkillTag(const FGameplayTag Tag) ;
	UFUNCTION()
	FORCEINLINE FGameplayTag& GetSpaceSkillTag() {return SpaceSkillTag;};
	
	UFUNCTION(BlueprintCallable,Server,Reliable)
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
	FORCEINLINE void ActivateBasicAttack(){ActivateTag(BasicAttackTag);}
	UFUNCTION()
	FORCEINLINE void ActivateQSkill(){ActivateTag(QSkillTag);}
	UFUNCTION()
	FORCEINLINE void ActivateESkill(){ActivateTag(ESkillTag);}
	UFUNCTION()
	FORCEINLINE void ActivateSpaceSkill(){ActivateTag(SpaceSkillTag);}
	
	
	//CallBacks
public:
	UFUNCTION()
	void QSkillCoolTime(FGameplayTag Tag, int32 NewCount);
	UFUNCTION()
	void ESkillCoolTime(FGameplayTag Tag, int32 NewCount);
	UFUNCTION()
	void SpaceSkillCoolTime(FGameplayTag Tag, int32 NewCount);

	//Rep
	UFUNCTION()
	FORCEINLINE void OnRep_QSkillTag(){OnQSkillSet.Broadcast(QSkillTag);}
	UFUNCTION()
	void OnRep_ESkillTag(){OnESkillSet.Broadcast(ESkillTag);}
	UFUNCTION()
	void OnRep_SpaceSkillTag(){OnSpaceSkillSet.Broadcast(SpaceSkillTag);}
	UFUNCTION()
	void OnRep_QSkillCoolTimeTag();
	UFUNCTION()
	void OnRep_ESkillCoolTimeTag();
	UFUNCTION()
	void OnRep_SpaceSkillCoolTimeTag();
	
	
	//Activate Skill By Tag
protected:
	void ActivateTag(FGameplayTag& Tag);
	
	float CalculateCoolTime(FGameplayTag& Tag);
	float CalculateMaxCoolTime(FGameplayTag& Tag);
	bool EnsureAbilitySystemComponent();
	
	
};
