// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDPlayerController.generated.h"

struct FInputActionValue;
class AEDCameraActor;
class AEDCursorActor;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;

DECLARE_DELEGATE_OneParam(FOnOtherInput,FInputActionValue);

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API AEDPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	AEDPlayerController();
	
	protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	
public:	
#pragma region Input Player
	//IMC_Player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputMappingContext> PlayerInputMappingContext=nullptr;
  // IA_Player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> MoveAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> LookAction=nullptr;
#pragma endregion
#pragma region Input Camera
	//IMC_Camera
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Camera")
	TObjectPtr<UInputMappingContext> CameraInputMappingContext=nullptr;
	// IA_Camera
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Camera")
	TObjectPtr<UInputAction> WheelAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Camera")
	TObjectPtr<UInputAction> KeyboardCAction=nullptr;
#pragma endregion
#pragma region Input Bindings
public:
	UFUNCTION()
	void CameraZoom(const FInputActionValue& value);
	
	UFUNCTION()
	void CameraFocus(const FInputActionValue& value);
	
	UFUNCTION()
	void CameraMove(const FInputActionValue& value);
	
#pragma endregion
#pragma region Spawn Actor
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actor")
	TSubclassOf<AEDCursorActor> CursorActorClass;
	UPROPERTY()
	TObjectPtr<AEDCursorActor> CursorActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actor")
	TSubclassOf<AEDCameraActor> CameraActorClass;
	UPROPERTY()
	TObjectPtr<AEDCameraActor> CameraActor;
#pragma endregion
#pragma region Delegate
	FOnOtherInput OnCameraScroll;
	FOnOtherInput OnCameraFocus;
	FOnOtherInput OnCameraMove;
#pragma endregion
	

};
