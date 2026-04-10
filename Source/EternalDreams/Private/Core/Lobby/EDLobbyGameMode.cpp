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
	int32 Counts[EDTeam::PlayerTeamCount] = {};

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return EDTeam::TeamA;

	for (APlayerState* BasePS : GS->PlayerArray)
	{
		AEDPlayerState* PS = Cast<AEDPlayerState>(BasePS);
		if (!PS) continue;

		for (int32 i = 0; i < EDTeam::PlayerTeamCount; ++i)
		{
			if (PS->TeamId == EDTeam::PlayerTeams[i])
			{
				Counts[i]++;
				break;
			}
		}
	}

	// 가장 인원 적은 팀 반환
	int32 MinIndex = 0;
	for (int32 i = 1; i < EDTeam::PlayerTeamCount; ++i)
	{
		if (Counts[i] < Counts[MinIndex])
		{
			MinIndex = i;
		}
	}

	return EDTeam::PlayerTeams[MinIndex];
}
