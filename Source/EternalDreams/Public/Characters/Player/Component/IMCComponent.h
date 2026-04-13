// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IMCComponent.generated.h"


struct FInputActionValue;
class UWidgetComponent;
class AEDPlayerController;
class AEDPlayerCharacter;

DECLARE_DELEGATE(FOnSkillInput);
/**
 * 캐릭터에 관한 인풋을 받아 적용하는 액터 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UIMCComponent : public UActorComponent
{
	GENERATED_BODY()

public:


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	

	//콜백 함수
public:
	UFUNCTION()
	void SetupPlayerInput(UInputComponent* PlayerInputComponent);
	UFUNCTION()
	void OnStopTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	protected:
	bool bIsStop=false;
	
	//캐싱
protected:
	UPROPERTY()
	TObjectPtr<AEDPlayerCharacter> PlayerCharacter;
	UPROPERTY()
	TObjectPtr<AEDPlayerController> PlayerController;


	//IA
protected:
	UFUNCTION()
	void PlayerMove(const FInputActionValue& value);
	UFUNCTION()
	void PlayerLook(const FInputActionValue& value);
	UFUNCTION()
	void PlayerBasicAttack(const FInputActionValue& value);
	
	UFUNCTION()
	void PlayerQSkill(const FInputActionValue& value);
	UFUNCTION()
	void PlayerESkill(const FInputActionValue& value);
	UFUNCTION()
	void PlayerSpaceSkill(const FInputActionValue& value);

	//Delegate
public:

	FOnSkillInput OnBasicAttackInput;
	FOnSkillInput OnQSkillInput;
	FOnSkillInput OnESkillInput;
	FOnSkillInput OnSpaceSkillInput;
	
	
	

};
