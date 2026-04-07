// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/EDPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Characters/Player/OtherActor/EDCameraActor.h"
#include "Characters/Player/OtherActor/EDCursorActor.h"
#include "Components/WidgetComponent.h"


AEDPlayerController::AEDPlayerController()
{

}

void AEDPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		//Create Component
		CursorActor=GetWorld()->SpawnActor<AEDCursorActor>(CursorActorClass);
		CameraActor=GetWorld()->SpawnActor<AEDCameraActor>(CameraActorClass);
		SetViewTargetWithBlend(CameraActor);
		
		if (IsValid(CameraActor))
		{
			OnCameraScroll.BindUObject(CameraActor,&AEDCameraActor::CameraZoom);
			OnCameraFocus.BindUObject(CameraActor,&AEDCameraActor::ToggleCameraFocus);
			OnCameraMove.BindUObject(CameraActor,&AEDCameraActor::CameraMove);
		}
	
		
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CameraInputMappingContext, 0);  // Gameplay
		}

	}
}

void AEDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	if (UEnhancedInputComponent* InputComponents = Cast<UEnhancedInputComponent>(InputComponent))
	{
		InputComponents->BindAction(
			WheelAction,
			ETriggerEvent::Triggered,
			this,
			&AEDPlayerController::CameraZoom
			);
		InputComponents->BindAction(
			KeyboardCAction,
			ETriggerEvent::Started,
			this,
			&AEDPlayerController::CameraFocus
			);
		InputComponents->BindAction(
			LookAction,
				ETriggerEvent::Triggered,
				this,
				&AEDPlayerController::CameraMove
	);
	}
}

void AEDPlayerController::CameraZoom(const FInputActionValue& value)
{
	OnCameraScroll.ExecuteIfBound(value);
}

void AEDPlayerController::CameraFocus(const FInputActionValue& value)
{
	OnCameraFocus.ExecuteIfBound(value);
}

void AEDPlayerController::CameraMove(const FInputActionValue& value)
{
	OnCameraMove.ExecuteIfBound(value);
}


