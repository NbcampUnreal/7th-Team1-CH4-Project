// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IMCComponent.generated.h"


class AEDPlayerController;
class AEDPlayerCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UIMCComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UIMCComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//콜백 함수
public:
	UFUNCTION()
	void SetupPlayerInput(UInputComponent* PlayerInputComponent);
	
protected:
	UPROPERTY()
	TObjectPtr<AEDPlayerCharacter> PlayerCharacter;
	
	UPROPERTY()
	TObjectPtr<AEDPlayerController> PlayerController;
	
	UFUNCTION()
	void PlayerMove(const FInputActionValue& value);
	UFUNCTION()
	void PlayerLook(const FInputActionValue& value);

};
