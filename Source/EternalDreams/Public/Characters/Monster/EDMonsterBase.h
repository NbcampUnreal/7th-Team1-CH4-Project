// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Data/Types/EDMonsterTypes.h"
#include "EDMonsterBase.generated.h"

class UAbilitySystemComponent;
class UEDMonsterDataAsset;

UCLASS()
class ETERNALDREAMS_API AEDMonsterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEDMonsterBase();
	
	virtual  UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	
	UFUNCTION(BlueprintCallable, Category = "Data")
	void InitializeFromDataAsset(UEDMonsterDataAsset* InDataAsset);
	
	UFUNCTION(BlueprintCallable, Category= "Data")
	UEDMonsterDataAsset* GetDataAsset() const { return DataAsset; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_MonsterState();
	
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void SetMonsterState(EMonsterState NewState) { if (HasAuthority()) MonsterState = NewState; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
private:
	UPROPERTY()
	TObjectPtr<UEDMonsterDataAsset> DataAsset;

	UPROPERTY(ReplicatedUsing = OnRep_MonsterState)
	EMonsterState MonsterState;
};
