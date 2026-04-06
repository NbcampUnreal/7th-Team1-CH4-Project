// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDPlayerController_Temp.h"
#include "Core/EDGameMode.h"
#include "Kismet/GameplayStatics.h"

void AEDPlayerController_Temp::Server_RequestStartGame_Implementation()
{
	AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
		return;
	}

	GM->StartGame();
}

void AEDPlayerController_Temp::Server_RequestStartPhaseSequence_Implementation()
{
	AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	GM->StartPhaseSequence();
}

void AEDPlayerController_Temp::Server_RequestSkipPhase_Implementation()
{
	AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	GM->SkipToNextPhase();
}
