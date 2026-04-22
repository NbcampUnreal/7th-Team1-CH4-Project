// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameState.h"

#include "Core/EDGameDataSubsystem.h"
#include "Core/EDPlayerState.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

AEDGameState::AEDGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AEDGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEDGameState, CurrentPhase);
	DOREPLIFETIME(AEDGameState, PhaseRemainingTime);
	DOREPLIFETIME(AEDGameState, WinnerTeamId);
	DOREPLIFETIME(AEDGameState, EliminatedTeams);
}

void AEDGameState::SetWinnerTeamId(int32 NewTeamId)
{
	if (!HasAuthority()) return;
	WinnerTeamId = NewTeamId;
}

void AEDGameState::ResetMatchResult()
{
	if (!HasAuthority()) return;

	WinnerTeamId = EDTeam::None;
	EliminatedTeams.Reset();
}

void AEDGameState::AddEliminatedTeam(int32 TeamId)
{
	if (!HasAuthority()) return;
	EliminatedTeams.AddUnique(TeamId);
}

void AEDGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 클라이언트 화면에 Phase 디버그 정보 표시
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (CurrentPhase.IsValid() && GEngine)
		{
			const int32 Day = GetCurrentDay();
			const bool bNight = GetIsNight();
			const int32 Minutes = FMath::FloorToInt(PhaseRemainingTime / 60.f);
			const int32 Seconds = FMath::FloorToInt(FMath::Fmod(PhaseRemainingTime, 60.f));

			const FString DebugMsg = FString::Printf(
				TEXT("Day%d %s  %02d:%02d  [%s]"),
				Day, bNight ? TEXT("Night") : TEXT("Day"),
				Minutes, Seconds, *CurrentPhase.ToString());

			GEngine->AddOnScreenDebugMessage(1000, 0.f, FColor::Yellow, DebugMsg);
		}
	}
}

void AEDGameState::BeginPlay()
{
	Super::BeginPlay();
	UEDGameDataSubsystem* DS = UEDGameDataSubsystem::Get(this);
	if (!DS) return;
	if (!DS->IsDataReady()) DS->InitializeGameData();
}

// ============================================================
//  Phase 태그에서 일차/밤낮 파싱
//
//  태그 형식: "Phase.DayN.Day" 또는 "Phase.DayN.Night"
//  예: Phase.Day2.Night → Day=2, Night=true
// ============================================================

int32 AEDGameState::GetCurrentDay() const
{
	if (!CurrentPhase.IsValid()) return 0;

	const FString TagStr = CurrentPhase.ToString();

	// "Phase.Day2.Night" → "Day" 찾기 (오프셋 6 = "Phase." 이후부터)
	const int32 DayCharIndex = TagStr.Find(TEXT("Day"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 6);
	if (DayCharIndex == INDEX_NONE) return 0;

	// "Day" 바로 뒤 한 글자에서 숫자 추출: "Day2" → '2'
	const int32 NumIndex = DayCharIndex + 3;
	if (!TagStr.IsValidIndex(NumIndex)) return 0;

	const TCHAR DayChar = TagStr[NumIndex];
	if (DayChar >= TEXT('1') && DayChar <= TEXT('4'))
	{
		return DayChar - TEXT('0');
	}

	return 0;
}

bool AEDGameState::GetIsNight() const
{
	if (!CurrentPhase.IsValid()) return false;

	// 태그가 ".Night"로 끝나는지 확인
	return CurrentPhase.ToString().EndsWith(TEXT("Night"));
}

// ============================================================

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
