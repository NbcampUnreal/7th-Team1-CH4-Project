// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDPlayerController.generated.h"

class ACursorActor;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;

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
	TObjectPtr<UInputAction> UIBackAction = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UIActor")
	TSubclassOf<ACursorActor> CursorActorClass;
	UPROPERTY()
	TObjectPtr<ACursorActor> CursorActor;

private:
	// 작성자 : 김동주
	// 인벤토리 패널 열기/닫기 입력 처리
	void HandleToggleInventory();

	// ESC 입력 시 패널 닫기 또는 Pause 메뉴 열기 처리
	void HandleUIBack();
};
