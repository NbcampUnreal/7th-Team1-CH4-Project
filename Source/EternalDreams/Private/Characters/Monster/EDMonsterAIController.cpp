// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/EDMonsterAIController.h"


// Sets default values
AEDMonsterAIController::AEDMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AEDMonsterAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

ETeamAttitude::Type AEDMonsterAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	return Super::GetTeamAttitudeTowards(Other);
}

void AEDMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEDMonsterAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void AEDMonsterAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
}

void AEDMonsterAIController::OnPerceptionForgotten(AActor* Actor)
{
}


