// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameState.h"
#include "Net/UnrealNetwork.h"

AEDGameState::AEDGameState()
{
}

void AEDGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEDGameState, CurrentPhase);
	DOREPLIFETIME(AEDGameState, PhaseRemainingTime);
}

void AEDGameState::SetCurrentPhase(const FGameplayTag& NewPhase)
{
	if (!HasAuthority()) return;
	if (CurrentPhase == NewPhase) return;

	const FGameplayTag OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	OnGamePhaseChanged.Broadcast(OldPhase, CurrentPhase);
}

void AEDGameState::SetPhaseRemainingTime(float NewTime)
{
	if (!HasAuthority()) return;
	PhaseRemainingTime = FMath::Max(0.f, NewTime);
}

void AEDGameState::OnRep_CurrentPhase(const FGameplayTag& OldPhase)
{
	OnGamePhaseChanged.Broadcast(OldPhase, CurrentPhase);
}
