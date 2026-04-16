// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameMode.h"
#include "EternalDreams.h"
#include "Core/EDGameState.h"
#include "Core/EDPlayerState.h"
#include "Core/PlayerStart/EDPlayerStart.h"
#include "Characters/Player/EDPlayerController.h"
#include "Environment/EDRestrictedArea.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

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


	// ============================================================
	// [IOCP 전환 시 활성화] 토큰 검증 & PendingToken 저장
	// ============================================================
	// 향후 IOCP 로비에서 접속할 때:
	//   1. Options에서 token 파싱
	//   2. DediServerSubsystem->IsTokenAuthorized(Token) 검증
	//   3. DediServerSubsystem->StorePendingToken(Address, Token)
	//   4. 검증 실패 시 ErrorMessage 설정하여 거부
	// 현재는 구형 로비(SeamlessTravel) 사용 중이므로 토큰 검증 생략.
	// ============================================================
}

// ============================================================
//  PostLogin — 팀 배정 → 스폰 → Phase 시작 체크
// ============================================================

void AEDGameMode::PostLogin(APlayerController* NewPlayer)
{
	// ============================================================
	// [IOCP 전환 시 활성화] IOCP 데이터 기반 팀 배정
	// ============================================================
	// 향후 IOCP 로비 연결 시 Super::PostLogin 호출 전에 팀을 배정해야 한다.
	// Super::PostLogin → HandleStartingNewPlayer → RestartPlayer → FindPlayerStart 순서로
	// 스폰이 발생하므로, TeamId가 먼저 설정되어야 올바른 위치에 스폰된다.
	//
	// 구현 예시:
	//   if (UEDDediServerSubsystem* DediSub = GetGameInstance()->GetSubsystem<UEDDediServerSubsystem>())
	//   {
	//       FString Address = NewPlayer->GetNetConnection()->LowLevelGetRemoteAddress(true);
	//       FString Token = DediSub->ConsumePendingToken(Address);
	//       if (const auto* Info = DediSub->GetPlayerInfoByToken(Token))
	//       {
	//           AEDPlayerState* PS = NewPlayer->GetPlayerState<AEDPlayerState>();
	//           if (PS) PS->TeamId = Info->TeamId;
	//       }
	//   }
	// ============================================================

	// 구형 로비: SeamlessTravel로 접속 시 PostLogin이 아닌
	// HandleSeamlessTravelPlayer가 호출되므로, 여기는 IOCP 전용 경로.
	// 현재는 Super만 호출 (구형 로비에서는 이 함수 자체가 호출되지 않음).

	Super::PostLogin(NewPlayer);

	// [IOCP 전환 시 활성화] 전원 접속 감지 후 Phase 시작
	// TryStartPhaseSequence();
}

void AEDGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	// BeginPlay보다 먼저 호출될 수 있으므로 캐시가 비어있으면 선행 초기화
	if (ZonePlayerStartMap.Num() == 0)
	{
		CacheZonePlayerStarts();
	}

	AEDPlayerState* PS = C ? C->GetPlayerState<AEDPlayerState>() : nullptr;
	UE_LOG(LogEDCore, Warning, TEXT("[Spawn] SeamlessTravel — Player: %s, DesiredZoneId: %d"),
		PS ? *PS->GetPlayerName() : TEXT("null"), PS ? PS->DesiredZoneId : -1);

	if (C && C->GetPawn())
	{
		C->GetPawn()->Destroy();
		C->SetPawn(nullptr);
	}

	Super::HandleSeamlessTravelPlayer(C);
}

void AEDGameMode::BeginPlay()
{
	Super::BeginPlay();

	CacheZonePlayerStarts();
	InitRestrictedZones();

	// ============================================================
	// [비동기로드] 게임 에셋 로드 시작 위치
	// ============================================================
	// 향후 EDGameDataSubsystem를 활용하여 게임 에셋(UI, Item, Monster)을
	// 비동기로 로드하는 로직을 여기에 추가한다.
	//
	// 구현 예시:
	//   if (UEDGameDataSubsystem* DataSub = UEDGameDataSubsystem::Get(this))
	//   {
	//       DataSub->InitializeGameData();
	//   }
	// ============================================================

	// 구형 로비 호환: BeginPlay에서 바로 Phase 시작 (테스트용)
	// [IOCP 전환 시] 아래 블록을 제거하고, PostLogin의 TryStartPhaseSequence()를 활성화
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
//  금지구역 제어
// ============================================================

void AEDGameMode::InitRestrictedZones()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEDRestrictedArea::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AEDRestrictedArea* Zone = Cast<AEDRestrictedArea>(Actor);
		if (Zone && Zone->ZoneID > 0)
		{
			RestrictedAreaMap.Add(Zone->ZoneID, Zone);
		}
	}

	// 매 판마다 무작위 금지 순서
	for (auto& Pair : RestrictedAreaMap)
	{
		RestrictedZoneOrder.Add(Pair.Key);
	}

	for (int32 i = RestrictedZoneOrder.Num() - 1; i > 0; --i)
	{
		int32 j = FMath::RandRange(0, i);
		RestrictedZoneOrder.Swap(i, j);
	}
}

void AEDGameMode::ActivateRestrictedZone(int32 ZoneID)
{
	if (AEDRestrictedArea** Found = RestrictedAreaMap.Find(ZoneID))
	{
		(*Found)->ActivateZone();
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
	// [금지구역] 1차 금지구역 활성화 (4구역 → 3구역)
	if (RestrictedZoneOrder.IsValidIndex(0))
	{
		ActivateRestrictedZone(RestrictedZoneOrder[0]);
	}
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
	// [금지구역] 2차 금지구역 활성화 (3구역 → 2구역)
	if (RestrictedZoneOrder.IsValidIndex(1))
	{
		ActivateRestrictedZone(RestrictedZoneOrder[1]);
	}
}

// ============================================================
//  Day 4 — 최종 결전 (최종 안전구역)
// ============================================================

void AEDGameMode::OnDay4_DayStarted()
{
	// [금지구역] 3차 금지구역 활성화 (2구역 → 최종)
	if (RestrictedZoneOrder.IsValidIndex(2))
	{
		ActivateRestrictedZone(RestrictedZoneOrder[2]);
	}
	// RestrictedZoneOrder[3]이 최종 안전구역으로 남음

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

// ============================================================
//  Starting System — 구역(Zone)별 스폰 위치
// ============================================================

void AEDGameMode::CacheZonePlayerStarts()
{
	if (ZonePlayerStartMap.Num() > 0) return;

	OccupiedPlayerStarts.Empty();

	for (TActorIterator<AEDPlayerStart> It(GetWorld()); It; ++It)
	{
		AEDPlayerStart* Start = *It;
		if (Start && Start->ZoneId > 0)
		{
			ZonePlayerStartMap.FindOrAdd(Start->ZoneId).Add(Start);
		}
	}

	UE_LOG(LogEDCore, Warning, TEXT("[Spawn] Cache — %d개 구역, 총 포인트:"), ZonePlayerStartMap.Num());
	for (const auto& Pair : ZonePlayerStartMap)
	{
		UE_LOG(LogEDCore, Warning, TEXT("[Spawn]   Zone %d: %d개"), Pair.Key, Pair.Value.Num());
	}
}

AActor* AEDGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	const AEDPlayerState* PS = Player->GetPlayerState<AEDPlayerState>();
	const int32 ZoneId = PS ? PS->DesiredZoneId : 0;

	if (const TArray<AEDPlayerStart*>* ZoneStarts = ZonePlayerStartMap.Find(ZoneId))
	{
		TArray<AEDPlayerStart*> Available;
		for (AEDPlayerStart* Start : *ZoneStarts)
		{
			if (!OccupiedPlayerStarts.Contains(Start))
			{
				Available.Add(Start);
			}
		}

		if (Available.Num() > 0)
		{
			AEDPlayerStart* Chosen = Available[FMath::RandRange(0, Available.Num() - 1)];
			OccupiedPlayerStarts.Add(Chosen);

			UE_LOG(LogEDCore, Warning, TEXT("[Spawn] %s → Zone %d, 사용가능 %d/%d, 선택: %s"),
				PS ? *PS->GetPlayerName() : TEXT("?"), ZoneId,
				Available.Num(), ZoneStarts->Num(), *Chosen->GetName());
			return Chosen;
		}
	}

	UE_LOG(LogEDCore, Warning, TEXT("[Spawn] %s → Zone %d 실패, 기본 폴백"),
		PS ? *PS->GetPlayerName() : TEXT("?"), ZoneId);
	return Super::ChoosePlayerStart_Implementation(Player);
}

// ============================================================
//  [IOCP 전용] 전원 접속 감지 → Phase 시작
// ============================================================

void AEDGameMode::TryStartPhaseSequence()
{
	if (bPhaseSequenceActive) return;
	if (PhaseSequence.Num() == 0) return;

	// ============================================================
	// [IOCP 전환 시 활성화] 접속 인원 체크
	// ============================================================
	// 향후 구현:
	//   UEDDediServerSubsystem* DediSub = GetGameInstance()->GetSubsystem<UEDDediServerSubsystem>();
	//   const int32 ExpectedCount = DediSub ? DediSub->GetExpectedPlayerCount() : 0;
	//   const int32 CurrentCount = GetNumPlayers();
	//   if (ExpectedCount > 0 && CurrentCount < ExpectedCount)
	//   {
	//       UE_LOG(LogEDCore, Warning, TEXT("[GameMode] TryStartPhaseSequence — 대기 중: %d/%d"),
	//           CurrentCount, ExpectedCount);
	//       return;
	//   }
	//
	// [비동기로드] 데이터 로드 완료 체크
	//   if (UEDGameDataSubsystem* DataSub = UEDGameDataSubsystem::Get(this))
	//   {
	//       if (!DataSub->IsDataReady())
	//       {
	//           // 데이터 완료 대기 — OnAllDataLoaded 델리게이트에서 재호출
	//           return;
	//       }
	//   }
	// ============================================================

	StartPhaseSequence();
}
