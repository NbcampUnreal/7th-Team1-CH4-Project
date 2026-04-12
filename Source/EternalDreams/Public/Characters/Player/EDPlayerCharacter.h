// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "EDPlayerCharacter.generated.h"

class AEDPlayerController;
class UWidgetComponent;
class UEDBaseAttributeSet;
class UEDPlayerAttributeSet;
class UIMCComponent;
class UZoneDetectorComponent;
class UEDInventoryComponent;

/*
 * 플레이어 캐릭터 클래스
 */
UCLASS()
class ETERNALDREAMS_API AEDPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEDPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
	
public:
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	UEDPlayerAttributeSet* GetPlayerAttributeSet() const { return PlayerAttributeSet; }
	
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	UEDBaseAttributeSet* GetBaseAttributeSet() const { return BaseAttributeSet; }
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	
	//Components
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UIMCComponent> IMCComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UZoneDetectorComponent> ZoneDetector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDPlayerAttributeSet> PlayerAttributeSet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDBaseAttributeSet> BaseAttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeSet")
	TSubclassOf<UEDPlayerAttributeSet> BPPlayerAttributeSet;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeSet")
	TSubclassOf<UEDBaseAttributeSet> BPBaseAttributeSet;
	
	virtual void InitializeAbilitySystem();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;
	
};
