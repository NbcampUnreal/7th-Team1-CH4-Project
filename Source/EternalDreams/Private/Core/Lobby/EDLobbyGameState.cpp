// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyGameState.h"
#include "Core/EDPlayerState.h"
#include "Net/UnrealNetwork.h"

AEDLobbyGameState::AEDLobbyGameState()
{
	TeamCounts.SetNum(EDTeam::PlayerTeamCount);
}

void AEDLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEDLobbyGameState, TeamCounts);
}

void AEDLobbyGameState::UpdateTeamCounts()
{
	if (!HasAuthority()) return;

	TeamCounts.SetNum(EDTeam::PlayerTeamCount);
	for (int32& Count : TeamCounts)
	{
		Count = 0;
	}

	for (APlayerState* BasePS : PlayerArray)
	{
		AEDPlayerState* PS = Cast<AEDPlayerState>(BasePS);
		if (!PS) continue;

		for (int32 i = 0; i < EDTeam::PlayerTeamCount; ++i)
		{
			if (PS->TeamId == EDTeam::PlayerTeams[i])
			{
				TeamCounts[i]++;
				break;
			}
		}
	}
}
