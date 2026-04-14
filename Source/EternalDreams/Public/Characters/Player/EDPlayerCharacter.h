// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "EDPlayerCharacter.generated.h"

class UGameplayAbility;
class AEDWeapon;
class AEDPlayerController;
class UWidgetComponent;
class UEDBaseAttributeSet;
class UEDPlayerAttributeSet;
class UIMCComponent;
class UZoneDetectorComponent;
class UEDInventoryComponent;
class USkillComponent;

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
	virtual void Tick( float DeltaSeconds ) override;

public:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PostInitializeComponents() override;
	
public:
	//GAS Getter	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<USkillComponent> SkillComponent;
	
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;
	
	//AttributeSet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDPlayerAttributeSet> PlayerAttributeSet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDBaseAttributeSet> BaseAttributeSet;
	
	//Abilities for Grant
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	//Test Weapon
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AEDWeapon> WeaponClass;
	UPROPERTY()
	TObjectPtr<AEDWeapon> WeaponMesh;
	UPROPERTY()
	FName WeaponSocketName=FName("handslot_r");
	
	//Get Attribute
public:
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxHealth() const;
	
	//Initialize AS
	virtual void InitializeAbilitySystem();

	// Ability Grant
	void GiveDefaultAbilities();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;
	
	//Get Animation Movement Input
public:
	UFUNCTION()
	void StartAnimMove(float InDashSpeed, bool InbIsForward, bool InbIsZ );
	UFUNCTION()
	void StopAnimMove();
	
protected:
	UPROPERTY()
	bool bIsAnimMoving=false;
	UPROPERTY()
	float DashSpeed=0.f;
	UPROPERTY()
	bool bIsForward=false;
	UPROPERTY()
	bool bIsZ=false;
	UPROPERTY()
	FVector MoveVector=FVector::ZeroVector;
	UPROPERTY()
	FHitResult Hit;
	
};
