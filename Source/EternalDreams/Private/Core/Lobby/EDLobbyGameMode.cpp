// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyGameMode.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyGameState.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "GameFramework/GameState.h"

AEDLobbyGameMode::AEDLobbyGameMode()
{
	PlayerStateClass = AEDPlayerState::StaticClass();
	GameStateClass = AEDLobbyGameState::StaticClass();
	PlayerControllerClass = AEDLobbyPlayerController::StaticClass();

	bUseSeamlessTravel = true;
}

void AEDLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	if (bGameStarting)
	{
		ErrorMessage = TEXT("MatchStarting");
	}
}

void AEDLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer) return;

	AEDPlayerState* PS = NewPlayer->GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	PS->TeamId = GetTeamWithFewerPlayers();
	PS->bReady = false;

	if (AEDLobbyGameState* LobbyGS = GetGameState<AEDLobbyGameState>())
	{
		LobbyGS->UpdateTeamCounts();
	}
}

void AEDLobbyGameMode::TryStartGame()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	if (GS->PlayerArray.Num() == 0)
	{
		return;
	}

	if (bGameStarting)
	{
		return;
	}

	for (APlayerState* BasePS : GS->PlayerArray)
	{
		AEDPlayerState* PS = Cast<AEDPlayerState>(BasePS);
		if (!PS) continue;

		if (!PS->bReady)
		{
			return;
		}
	}

	bGameStarting = true;
	GetWorld()->ServerTravel(GameMapPath);
}

int32 AEDLobbyGameMode::GetTeamWithFewerPlayers() const
{
	int32 TeamACount = 0;
	int32 TeamBCount = 0;

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return 0;

	for (APlayerState* BasePS : GS->PlayerArray)
	{
		AEDPlayerState* PS = Cast<AEDPlayerState>(BasePS);
		if (!PS) continue;

		if (PS->TeamId == 0) TeamACount++;
		else if (PS->TeamId == 1) TeamBCount++;
	}

	return (TeamACount <= TeamBCount) ? 0 : 1;
}
