// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDPlayerState.h"
#include "Net/UnrealNetwork.h"

AEDPlayerState::AEDPlayerState()
{
}

void AEDPlayerState::SetDisplayNickname(const FString& InDisplayNickname)
{
	DisplayNickname = InDisplayNickname.TrimStartAndEnd();
}

void AEDPlayerState::SetPlayerSkinName_Implementation(EPlayerNameType InPlayerSkinName)
{
	PlayerSkinName=InPlayerSkinName;
}

void AEDPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEDPlayerState, TeamId);
	DOREPLIFETIME(AEDPlayerState, DisplayNickname);
	DOREPLIFETIME(AEDPlayerState, bReady);
	DOREPLIFETIME(AEDPlayerState, DesiredZoneId);
	DOREPLIFETIME(AEDPlayerState, bIsDead);
	DOREPLIFETIME(AEDPlayerState, bEliminated);
	DOREPLIFETIME(AEDPlayerState, RemainingRevives);
	DOREPLIFETIME(AEDPlayerState, Kills);
	DOREPLIFETIME(AEDPlayerState, Deaths);
	DOREPLIFETIME(AEDPlayerState, PlayerSkinName);
}

void AEDPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AEDPlayerState* PS = Cast<AEDPlayerState>(PlayerState))
	{
		PS->TeamId = TeamId;
		PS->DisplayNickname = DisplayNickname;
		PS->bReady = bReady;
		PS->DesiredZoneId = DesiredZoneId;
		PS->bIsDead = bIsDead;
		PS->bEliminated = bEliminated;
		PS->RemainingRevives = RemainingRevives;
		PS->Kills = Kills;
		PS->Deaths = Deaths;
		PS->PlayerSkinName = PlayerSkinName;
		PS->bHasSavedInventorySnapshot = bHasSavedInventorySnapshot;
		PS->SavedInventorySnapshot = SavedInventorySnapshot;
	}
}
