// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyGameMode.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyGameState.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "GameFramework/GameState.h"

namespace
{
	const TCHAR* NetModeToString(const UWorld* World)
	{
		if (!World)
		{
			return TEXT("NoWorld");
		}

		switch (World->GetNetMode())
		{
		case NM_Standalone:
			return TEXT("Standalone");
		case NM_DedicatedServer:
			return TEXT("DedicatedServer");
		case NM_ListenServer:
			return TEXT("ListenServer");
		case NM_Client:
			return TEXT("Client");
		default:
			return TEXT("Unknown");
		}
	}
}

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
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] PreLogin rejected | Address=%s | Reason=%s"), *Address, *ErrorMessage);
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

	UE_LOG(LogTemp, Log, TEXT("[LobbyGameMode] PostLogin | Player=%s | Team=%d | PlayerCount=%d | Map=%s | NetMode=%s"),
		*PS->GetPlayerName(),
		PS->TeamId,
		GameState ? GameState->PlayerArray.Num() : -1,
		GetWorld() ? *GetWorld()->GetMapName() : TEXT("None"),
		NetModeToString(GetWorld()));

	if (AEDLobbyGameState* LobbyGS = GetGameState<AEDLobbyGameState>())
	{
		LobbyGS->UpdateTeamCounts();
	}
}

void AEDLobbyGameMode::TryStartGame()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	UE_LOG(LogTemp, Log, TEXT("[LobbyGameMode] TryStartGame called | PlayerCount=%d | Map=%s | NetMode=%s"),
		GS->PlayerArray.Num(),
		GetWorld() ? *GetWorld()->GetMapName() : TEXT("None"),
		NetModeToString(GetWorld()));

	if (GS->PlayerArray.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] TryStartGame - No players connected"));
		return;
	}

	if (bGameStarting)
	{
		UE_LOG(LogTemp, Log, TEXT("[LobbyGameMode] TryStartGame ignored - match is already starting"));
		return;
	}

	for (APlayerState* BasePS : GS->PlayerArray)
	{
		AEDPlayerState* PS = Cast<AEDPlayerState>(BasePS);
		if (!PS) continue;

		UE_LOG(LogTemp, Log, TEXT("[LobbyGameMode] ReadyCheck | Player=%s | Team=%d | Ready=%s"),
			*PS->GetPlayerName(),
			PS->TeamId,
			PS->bReady ? TEXT("true") : TEXT("false"));

		if (!PS->bReady)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LobbyGameMode] TryStartGame - %s is not ready"), *PS->GetPlayerName());
			return;
		}
	}

	bGameStarting = true;
	UE_LOG(LogTemp, Log, TEXT("[LobbyGameMode] All players ready. ServerTravel to %s"), *GameMapPath);
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
