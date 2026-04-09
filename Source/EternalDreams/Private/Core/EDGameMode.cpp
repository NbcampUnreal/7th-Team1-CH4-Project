// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameMode.h"
#include "Core/EDGameState.h"
#include "Core/EDPlayerState.h"
#include "Characters/Player/EDPlayerController.h"

AEDGameMode::AEDGameMode()
{
	GameStateClass        = AEDGameState::StaticClass();
	PlayerStateClass      = AEDPlayerState::StaticClass();
	PlayerControllerClass = AEDPlayerController::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	bUseSeamlessTravel = true;
}

// ============================================================
//  UE 오버라이드
// ============================================================

void AEDGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	ErrorMessage = TEXT("MatchAlreadyStarted");
}

void AEDGameMode::BeginPlay()
{
	Super::BeginPlay();

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

	if (AEDGameState* GS = GetGameState<AEDGameState>())
	{
		GS->SetPhaseRemainingTime(PhaseTimer);
	}

	if (PhaseTimer <= 0.f)
	{
		SkipToNextPhase();
	}
}

// ============================================================
//  Phase 제어
// ============================================================

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
		bPhaseSequenceActive = false;
		PhaseTimer = 0.f;

		if (AEDGameState* GS = GetGameState<AEDGameState>())
		{
			GS->SetPhaseRemainingTime(0.f);
		}

		OnMatchFinished();
		return;
	}

	AdvanceToPhase(NextIndex);
}

FGameplayTag AEDGameMode::GetCurrentPhase() const
{
	if (const AEDGameState* GS = GetGameState<AEDGameState>())
	{
		return GS->GetCurrentPhase();
	}
	return FGameplayTag();
}

int32 AEDGameMode::GetCurrentDay() const
{
	if (const AEDGameState* GS = GetGameState<AEDGameState>())
	{
		return GS->GetCurrentDay();
	}
	return 0;
}

bool AEDGameMode::IsNight() const
{
	if (const AEDGameState* GS = GetGameState<AEDGameState>())
	{
		return GS->GetIsNight();
	}
	return false;
}

// ============================================================
//  Phase 내부
// ============================================================

void AEDGameMode::AdvanceToPhase(int32 PhaseIndex)
{
	CurrentPhaseIndex = PhaseIndex;
	PhaseTimer = GetPhaseDuration(PhaseIndex);

	SetPhase(PhaseSequence[PhaseIndex]);

	if (AEDGameState* GS = GetGameState<AEDGameState>())
	{
		GS->SetPhaseRemainingTime(PhaseTimer);
	}

	OnPhaseStarted(PhaseIndex, PhaseSequence[PhaseIndex]);
}

float AEDGameMode::GetPhaseDuration(int32 PhaseIndex) const
{
	if (PhaseDurations.IsValidIndex(PhaseIndex))
	{
		return PhaseDurations[PhaseIndex];
	}
	return DefaultPhaseDuration;
}

void AEDGameMode::SetPhase(FGameplayTag NewPhase)
{
	if (AEDGameState* GS = GetGameState<AEDGameState>())
	{
		GS->SetCurrentPhase(NewPhase);
	}
}

// ============================================================
//  Phase 진입 콜백 — 메인 분기점
// ============================================================
//  각 Phase에서 트리거해야 할 시스템을 아래 함수에 추가한다.


void AEDGameMode::OnPhaseStarted(int32 PhaseIndex, const FGameplayTag& PhaseTag)
{
	switch (PhaseIndex)
	{
	case 0: OnDay1_DayStarted();   break;
	case 1: OnDay1_NightStarted(); break;
	case 2: OnDay2_DayStarted();   break;
	case 3: OnDay2_NightStarted(); break;
	case 4: OnDay3_DayStarted();   break;
	case 5: OnDay3_NightStarted(); break;
	case 6: OnDay4_DayStarted();   break;
	case 7: OnDay4_NightStarted(); break;
	default: break;
	}
}

// ============================================================
//  Day 1 — 초반 성장 (구역 4/4)
// ============================================================

void AEDGameMode::OnDay1_DayStarted()
{
	// [몬스터] 기본 몬스터 스폰 시작 (30초 주기) — S2 담당
	// [아이템] 기본 재료 필드 배치 — S4 담당
}

void AEDGameMode::OnDay1_NightStarted()
{
	// [몬스터] 밤 몬스터 강화/추가 스폰 — S2 담당
}

// ============================================================
//  Day 2 — 중반 경쟁 (구역 3/4)
// ============================================================

void AEDGameMode::OnDay2_DayStarted()
{
	// [아이템] 에픽 등급 재료 등장 — S4 담당
	// [부활] 부활키트 사용 가능 시작 — S6 담당
}

void AEDGameMode::OnDay2_NightStarted()
{
	// [금지구역] 1차 금지구역 경고 → 활성화 (4구역 → 3구역) — S6 담당
}

// ============================================================
//  Day 3 — 후반 격돌 (구역 2/4)
// ============================================================

void AEDGameMode::OnDay3_DayStarted()
{
	// [몬스터] 위클라이너(보스) 스폰 — S2 담당
	// [아이템] 전설 등급 재료 등장 — S4 담당
	// [부활] 골드 환급 방식으로 변경 — S6 담당
}

void AEDGameMode::OnDay3_NightStarted()
{
	// [금지구역] 2차 금지구역 경고 → 활성화 (3구역 → 2구역) — S6 담당
}

// ============================================================
//  Day 4 — 최종 결전 (최종 안전구역)
// ============================================================

void AEDGameMode::OnDay4_DayStarted()
{
	// [금지구역] 최종 안전구역 수렴 — S6 담당
	// [부활] 부활 불가 — S6 담당
}

void AEDGameMode::OnDay4_NightStarted()
{
	// [최종결전] 안전구역 밟기 판정 시작 — S6 담당
	//   - 같이 밟으면 시간 동시 감소
	//   - 한 팀만 밟으면 상대 시간만 감소
	//   - 시간 많은 팀 승리
}

// ============================================================
//  매치 종료
// ============================================================

void AEDGameMode::OnMatchFinished()
{
	// [승패] 최종 승패 판정 & 결과 UI 표시 — S3/S6 담당
	// [세션] 로비 복귀 또는 세션 정리 — S6 담당
}
