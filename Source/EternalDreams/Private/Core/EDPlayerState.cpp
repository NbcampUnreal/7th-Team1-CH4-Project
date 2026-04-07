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
	DOREPLIFETIME(AEDPlayerState, bReady);
}

void AEDPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AEDPlayerState* PS = Cast<AEDPlayerState>(PlayerState))
	{
		PS->TeamId = TeamId;
		PS->bReady = bReady;
	}
}
