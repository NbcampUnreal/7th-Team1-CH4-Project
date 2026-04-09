// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyGameMode.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyGameState.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "GameFramework/GameState.h"

DEFINE_LOG_CATEGORY_STATIC(LogEDLobby, Log, All);

AEDLobbyGameMode::AEDLobbyGameMode()
{
	PlayerStateClass = AEDPlayerState::StaticClass();
	GameStateClass = AEDLobbyGameState::StaticClass();
	PlayerControllerClass = AEDLobbyPlayerController::StaticClass();

	bUseSeamlessTravel = true;
}

void AEDLobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	UE_LOG(LogEDLobby, Log, TEXT("PreLogin requested. Address=%s Options=%s"), *Address, *Options);
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG(LogEDLobby, Warning, TEXT("PreLogin rejected. Address=%s Error=%s"), *Address, *ErrorMessage);
		return;
	}

	if (bGameStarting)
	{
		ErrorMessage = TEXT("MatchStarting");
		UE_LOG(LogEDLobby, Warning, TEXT("PreLogin rejected because match is starting. Address=%s"), *Address);
		return;
	}

	UE_LOG(LogEDLobby, Log, TEXT("PreLogin accepted. Address=%s"), *Address);
}

void AEDLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer) return;

	AEDPlayerState* PS = NewPlayer->GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	PS->TeamId = GetTeamWithFewerPlayers();
	PS->bReady = false;

	UE_LOG(LogEDLobby, Log, TEXT("PostLogin complete. Player=%s TeamId=%d PlayerCount=%d"),
		*PS->GetPlayerName(), PS->TeamId, GameState ? GameState->PlayerArray.Num() : -1);

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
			UE_LOG(LogEDLobby, Log, TEXT("TryStartGame waiting. Player=%s TeamId=%d Ready=%d"),
				*PS->GetPlayerName(), PS->TeamId, PS->bReady ? 1 : 0);
			return;
		}
	}

	bGameStarting = true;
	UE_LOG(LogEDLobby, Log, TEXT("All players ready. Starting ServerTravel to %s"), *GameMapPath);
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
