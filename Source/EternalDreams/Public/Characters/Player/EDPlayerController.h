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

	// 작성자 : 김동주
	// Enhanced Input 액션을 실제 처리 함수에 바인딩
	void SetupInputComponent();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction = nullptr;

	// 작성자 : 김동주
	// 인벤토리 패널 열기/닫기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> ToggleInventoryAction = nullptr;

	// ESC 입력 처리용 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	
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
	
	//Actors
	TObjectPtr<UInputAction> UIBackAction = nullptr;

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
	TObjectPtr<ACursorActor> CursorActor;

private:
	// 작성자 : 김동주
	// 인벤토리 패널 열기/닫기 입력 처리
	void HandleToggleInventory();

	// ESC 입력 시 패널 닫기 또는 Pause 메뉴 열기 처리
	void HandleUIBack();

	// 애플리케이션 복귀 시 현재 열린 UI 상태에 맞게 입력 모드와 포커스 복구를 요청
	void HandleApplicationReactivated();
#pragma endregion
#pragma region Delegate
	FOnOtherInput OnCameraScroll;
	FOnOtherInput OnCameraFocus;
	FOnOtherInput OnCameraMove;
#pragma endregion
	

};
