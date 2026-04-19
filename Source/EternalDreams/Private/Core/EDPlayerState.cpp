// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDPlayerState.h"
#include "Net/UnrealNetwork.h"

AEDPlayerState::AEDPlayerState()
{
}

void AEDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEDPlayerState, TeamId);
	DOREPLIFETIME(AEDPlayerState, DesiredZoneId);
	DOREPLIFETIME(AEDPlayerState, bIsDead);
	DOREPLIFETIME(AEDPlayerState, bEliminated);
	DOREPLIFETIME(AEDPlayerState, RemainingRevives);
	DOREPLIFETIME(AEDPlayerState, Kills);
	DOREPLIFETIME(AEDPlayerState, Deaths);
}
