// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/EDPlayerController.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Characters/Player/CursorActor.h"
#include "Components/WidgetComponent.h"


AEDPlayerController::AEDPlayerController()
{

}

void AEDPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		CursorActor=GetWorld()->SpawnActor<ACursorActor>(CursorActorClass);
	}
}

