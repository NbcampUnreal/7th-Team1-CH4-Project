// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameState.h"
#include "Net/UnrealNetwork.h"

AEDGameState::AEDGameState()
{
}

void AEDGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
