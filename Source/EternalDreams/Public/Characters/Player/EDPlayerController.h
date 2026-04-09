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
 * 플레이어 컨트롤러 클래스
 */
UCLASS()
class ETERNALDREAMS_API AEDPlayerController : public APlayerController
{
	GENERATED_BODY()

	AEDPlayerController();

protected:
	virtual void BeginPlay() override;
		// 작성자 : 김동주
    	// Enhanced Input 액션을 실제 처리 함수에 바인딩
	virtual void SetupInputComponent() override;



public:
#pragma region Input UI
	// UI 입력 전용 매핑 컨텍스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI")
	TObjectPtr<UInputMappingContext> UIInputMappingContext = nullptr;
	
	// 인벤토리 패널 열기/닫기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI")
	TObjectPtr<UInputAction> ToggleInventoryAction = nullptr;

	// ESC 입력 처리용 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI")
	TObjectPtr<UInputAction> UIBackAction = nullptr;
#pragma endregion 김동주
	
	// -------------------------------------------------------
	// 테스트용 RPC (페이즈)
	// -------------------------------------------------------

	/** 클라이언트에서 호출 → 서버에서 실행. 페이즈 시퀀스 시작 요청. (테스트용) */
	UFUNCTION(Server, Reliable)
	void Server_RequestStartPhaseSequence();

	/** 클라이언트에서 호출 → 서버에서 실행. 다음 페이즈 스킵 요청. (테스트용) */
	UFUNCTION(Server, Reliable)
	void Server_RequestSkipPhase();

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
private:
#pragma region UI 
	// 인벤토리 패널 열기/닫기 입력 처리
	void HandleToggleInventory();

	// ESC 입력 시 패널 닫기 또는 Pause 메뉴 열기 처리
	void HandleUIBack();

	// 애플리케이션 복귀 시 현재 열린 UI 상태에 맞게 입력 모드와 포커스 복구를 요청
	void HandleApplicationReactivated();
#pragma endregion 김동주

#pragma region Delegate
	FOnOtherInput OnCameraScroll;
	FOnOtherInput OnCameraFocus;
#pragma endregion
	

};
