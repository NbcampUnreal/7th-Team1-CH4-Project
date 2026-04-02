// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameMode.h"
#include "Core/EDGameState.h"
#include "Core/EDPlayerState.h"
#include "Core/EDPlayerController_Temp.h"

AEDGameMode::AEDGameMode()
{
	GameStateClass        = AEDGameState::StaticClass();
	PlayerStateClass      = AEDPlayerState::StaticClass();
	PlayerControllerClass = AEDPlayerController_Temp::StaticClass();
}

void AEDGameMode::StartGame()
{
	GetWorld()->ServerTravel(TEXT("/Game/ED/Asset/IgnoredAsset/Test/GameMap"));
}
