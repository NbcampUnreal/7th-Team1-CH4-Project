// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyGameMode.h"
#include "Core/Lobby/EDLobbyGameState.h"
#include "Kismet/GameplayStatics.h"

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

void AEDLobbyPlayerController::Server_SetReady_Implementation()
{
	AEDPlayerState* PS = GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	PS->bReady = !PS->bReady;

	UE_LOG(LogTemp, Log, TEXT("[LobbyPC] Server_SetReady | Player=%s | Ready=%s | Map=%s | NetMode=%s | HasAuthority=%s"),
		*PS->GetPlayerName(),
		PS->bReady ? TEXT("true") : TEXT("false"),
		GetWorld() ? *GetWorld()->GetMapName() : TEXT("None"),
		NetModeToString(GetWorld()),
		HasAuthority() ? TEXT("true") : TEXT("false"));

	// 팀 인원 갱신
	if (AEDLobbyGameState* LobbyGS = GetWorld()->GetGameState<AEDLobbyGameState>())
	{
		LobbyGS->UpdateTeamCounts();
	}

	AEDLobbyGameMode* GM = Cast<AEDLobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyPC] Server_SetReady - LobbyGameMode not found"));
		return;
	}

	GM->TryStartGame();
}
