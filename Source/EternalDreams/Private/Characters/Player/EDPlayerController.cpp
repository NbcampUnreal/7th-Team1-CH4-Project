// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/EDPlayerController.h"

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
		CursorActor=GetWorld()->SpawnActor<AEDCursorActor>(CursorActorClass);
		CameraActor=GetWorld()->SpawnActor<AEDCameraActor>(CameraActorClass);
		SetViewTargetWithBlend(CameraActor);

	}
}

