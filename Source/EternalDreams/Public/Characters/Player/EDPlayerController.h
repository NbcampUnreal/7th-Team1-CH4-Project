// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDPlayerController.generated.h"

class AEDCameraActor;
class AEDCursorActor;
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

	
public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext=nullptr;
  // IMC_UI (ESC, Inventory - 항상 활성)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> MoveAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction=nullptr;
	
	//Actors
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actor")
	TSubclassOf<AEDCursorActor> CursorActorClass;
	UPROPERTY()
	TObjectPtr<AEDCursorActor> CursorActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actor")
	TSubclassOf<AEDCameraActor> CameraActorClass;
	UPROPERTY()
	TObjectPtr<AEDCameraActor> CameraActor;
};
