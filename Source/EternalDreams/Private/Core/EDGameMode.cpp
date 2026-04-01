// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameMode.h"
#include "Core/EDGameState.h"
#include "Core/EDPlayerState.h"

AEDGameMode::AEDGameMode()
{
	GameStateClass   = AEDGameState::StaticClass();
	PlayerStateClass = AEDPlayerState::StaticClass();
}
