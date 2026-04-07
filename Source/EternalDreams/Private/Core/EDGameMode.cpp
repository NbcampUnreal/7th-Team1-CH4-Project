// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameMode.h"
#include "Core/EDGameState.h"
#include "Core/EDPlayerState.h"
#include "Core/EDPlayerController_Temp.h"

AEDGameMode::AEDGameMode()
{
	GameStateClass        = AEDGameState::StaticClass();
	PlayerStateClass      = AEDPlayerState::StaticClass();
	PlayerControllerClass = AEDPlayerController_Temp::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void AEDGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 데디케이티드 서버: GameMode는 서버에서만 존재
	// PhaseSequence가 설정되어 있으면 자동으로 페이즈 시퀀스 시작
	if (PhaseSequence.Num() > 0)
	{
		StartPhaseSequence();
	}
}

void AEDGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bPhaseSequenceActive) return;

	PhaseTimer -= DeltaSeconds;

	// GameState에 남은 시간 동기화
	if (AEDGameState* GS = GetGameState<AEDGameState>())
	{
		GS->SetPhaseRemainingTime(PhaseTimer);
	}

	if (PhaseTimer <= 0.f)
	{
		SkipToNextPhase();
	}
}

void AEDGameMode::StartGame()
{
	GetWorld()->ServerTravel(GameMapPath);
}

void AEDGameMode::SetPhase(FGameplayTag NewPhase)
{
	if (AEDGameState* GS = GetGameState<AEDGameState>())
	{
		GS->SetCurrentPhase(NewPhase);
	}
}

FGameplayTag AEDGameMode::GetCurrentPhase() const
{
	if (const AEDGameState* GS = GetGameState<AEDGameState>())
	{
		return GS->GetCurrentPhase();
	}
	return FGameplayTag();
}

void AEDGameMode::StartPhaseSequence()
{
	if (PhaseSequence.Num() == 0)
	{
		return;
	}

	bPhaseSequenceActive = true;
	AdvanceToPhase(0);

}

void AEDGameMode::SkipToNextPhase()
{
	const int32 NextIndex = CurrentPhaseIndex + 1;

	if (NextIndex >= PhaseSequence.Num())
	{
		// 마지막 페이즈 종료
		bPhaseSequenceActive = false;
		PhaseTimer = 0.f;

		if (AEDGameState* GS = GetGameState<AEDGameState>())
		{
			GS->SetPhaseRemainingTime(0.f);
		}

		return;
	}

	AdvanceToPhase(NextIndex);
}

void AEDGameMode::AdvanceToPhase(int32 PhaseIndex)
{
	CurrentPhaseIndex = PhaseIndex;
	PhaseTimer = GetPhaseDuration(PhaseIndex);

	SetPhase(PhaseSequence[PhaseIndex]);

	if (AEDGameState* GS = GetGameState<AEDGameState>())
	{
		GS->SetPhaseRemainingTime(PhaseTimer);
	}

}

float AEDGameMode::GetPhaseDuration(int32 PhaseIndex) const
{
	if (PhaseDurations.IsValidIndex(PhaseIndex))
	{
		return PhaseDurations[PhaseIndex];
	}
	return DefaultPhaseDuration;
}
