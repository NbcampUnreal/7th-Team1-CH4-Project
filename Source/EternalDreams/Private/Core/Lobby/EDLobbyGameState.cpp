// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyGameState.h"
#include "Core/EDPlayerState.h"
#include "Net/UnrealNetwork.h"

AEDLobbyGameState::AEDLobbyGameState()
{
}

void AEDLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEDLobbyGameState, TeamACount);
	DOREPLIFETIME(AEDLobbyGameState, TeamBCount);
}

void AEDLobbyGameState::UpdateTeamCounts()
{
	if (!HasAuthority()) return;

	int32 CountA = 0;
	int32 CountB = 0;

	for (APlayerState* BasePS : PlayerArray)
	{
		AEDPlayerState* PS = Cast<AEDPlayerState>(BasePS);
		if (!PS) continue;

		if (PS->TeamId == 0) CountA++;
		else if (PS->TeamId == 1) CountB++;
	}

	TeamACount = CountA;
	TeamBCount = CountB;
}
