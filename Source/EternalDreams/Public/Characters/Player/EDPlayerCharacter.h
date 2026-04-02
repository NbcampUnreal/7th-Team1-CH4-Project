// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "EDPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UEDBaseAttributeSet;
class UEDPlayerAttributeSet;
/*
 * [EDPlayerCharacter]
 * 체력, 방어력, 이동속도의 현재와 최대치(혹은 감소되기 전 값) Attribute를 관리하는 캐릭터, 몬스터의 공통된 AttributeSet
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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
	
public:
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	UEDPlayerAttributeSet* GetPlayerAttributeSet() const { return PlayerAttributeSet; }
	
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	UEDBaseAttributeSet* GetBaseAttributeSet() const { return BaseAttributeSet; }
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpringArmLength=1000.f;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDPlayerAttributeSet> PlayerAttributeSet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDBaseAttributeSet> BaseAttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeSet")
	TSubclassOf<UEDPlayerAttributeSet> BPPlayerAttributeSet;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AttributeSet")
	TSubclassOf<UEDBaseAttributeSet> BPBaseAttributeSet;
	
	virtual void InitializeAbilitySystem();

};
