// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyGameMode.h"
#include "Core/Lobby/EDLobbyGameState.h"
#include "Kismet/GameplayStatics.h"

void AEDLobbyPlayerController::Server_SetReady_Implementation()
{
	AEDPlayerState* PS = GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	PS->bReady = !PS->bReady;

	if (AEDLobbyGameState* LobbyGS = GetWorld()->GetGameState<AEDLobbyGameState>())
	{
		LobbyGS->UpdateTeamCounts();
	}

	AEDLobbyGameMode* GM = Cast<AEDLobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	GM->TryStartGame();
}

void AEDLobbyPlayerController::Server_ChangeTeam_Implementation(int32 NewTeamId)
{
	if (!EDTeam::IsPlayerTeam(NewTeamId)) return;

	AEDPlayerState* PS = GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	if (PS->TeamId == NewTeamId) return;

	PS->TeamId = NewTeamId;
	PS->bReady = false;

	if (AEDLobbyGameState* LobbyGS = GetWorld()->GetGameState<AEDLobbyGameState>())
	{
		LobbyGS->UpdateTeamCounts();
	}
}
