// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Data/Types/EDMonsterTypes.h"
#include "EDMonsterBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnAttackFinished);

class UAbilitySystemComponent;
class UEDMonsterDataAsset;
struct FStreamableHandle;

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
	
	FVector GetOriginLocation() const { return OriginLocation;}
	
	FOnAttackFinished OnAttackFinished;
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TObjectPtr<UEDMonsterDataAsset> DataAsset;
private:
	// 비동기 로드(임시)
	void LoadVisuals(UEDMonsterDataAsset* InDataAsset);
	// 비동기 로드 완료 콜백
	void OnVisualsLoaded();
	
	UPROPERTY(ReplicatedUsing = OnRep_MonsterState)
	EMonsterState MonsterState;
	
	UPROPERTY()
	TObjectPtr<UEDBaseAttributeSet> BaseAttributeSet;
	
	FVector OriginLocation;
	TSharedPtr<FStreamableHandle> VisualLoadHandle;
};
