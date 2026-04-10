// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyGameMode.h"
#include "EternalDreams.h"
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
	UE_LOG(LogEDCore, Warning, TEXT("[LobbyGM] PreLogin — Address: %s, Options: %s"), *Address, *Options);

	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG(LogEDCore, Error, TEXT("[LobbyGM] PreLogin 거부(Super) — Address: %s, Error: %s"), *Address, *ErrorMessage);
		return;
	}

	if (bGameStarting)
	{
		ErrorMessage = TEXT("MatchStarting");
		UE_LOG(LogEDCore, Warning, TEXT("[LobbyGM] PreLogin 거부 — 게임 시작 중, Address: %s"), *Address);
	}
}

void AEDLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		UE_LOG(LogEDCore, Error, TEXT("[LobbyGM] PostLogin — NewPlayer가 null"));
		return;
	}

	AEDPlayerState* PS = NewPlayer->GetPlayerState<AEDPlayerState>();
	if (!PS)
	{
		UE_LOG(LogEDCore, Error, TEXT("[LobbyGM] PostLogin — PlayerState가 null, Player: %s"), *NewPlayer->GetName());
		return;
	}

	PS->TeamId = GetTeamWithFewerPlayers();
	PS->bReady = false;

	UE_LOG(LogEDCore, Warning, TEXT("[LobbyGM] PostLogin 성공 — Player: %s, TeamId: %d"), *NewPlayer->GetName(), PS->TeamId);

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
			UE_LOG(LogEDCore, Warning, TEXT("[LobbyGM] TryStartGame — %s 아직 Ready 아님"), *PS->GetPlayerName());
			return;
		}
	}

	bGameStarting = true;
	UE_LOG(LogEDCore, Warning, TEXT("[LobbyGM] TryStartGame — 전원 Ready! ServerTravel 시작, GameMapPath: %s"), *GameMapPath);
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
