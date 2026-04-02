// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDPlayerController_Temp.h"
#include "Core/EDGameMode.h"
#include "Kismet/GameplayStatics.h"

void AEDPlayerController_Temp::Server_RequestStartGame_Implementation()
{
	AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerController_Temp] Server_RequestStartGame 실패: GameMode nullptr."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerController_Temp] Server_RequestStartGame → GameMode::StartGame() 호출."));
	GM->StartGame();
}
