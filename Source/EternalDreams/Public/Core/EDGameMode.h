// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameMode.h"
#include "EDGameMode.generated.h"

class AEDRestrictedArea;
class AEDPlayerStart;

/**
 * AEDGameMode
 *
 * 인게임 매치 플로우를 관리한다.
 * 4일 사이클 (각 일차: 낮 → 밤), 총 8개 Phase.
 *
 * ============================================================
 *  Phase 흐름 (총 ~12분)
 * ============================================================
 *
 *  Phase 0: Day1 낮  (90초)  - 탐색 & 기본 파밍
 *  Phase 1: Day1 밤  (90초)  - 기본 몬스터 스폰
 *  Phase 2: Day2 낮  (90초)  - 에픽 재료 등장
 *  Phase 3: Day2 밤  (90초)  - 1차 금지구역 활성화
 *  Phase 4: Day3 낮  (90초)  - 위클라이너(보스) 스폰, 전설 재료
 *  Phase 5: Day3 밤  (90초)  - 2차 금지구역 활성화
 *  Phase 6: Day4 낮  (90초)  - 최종 안전구역 수렴
 *  Phase 7: Day4 밤  (90초)  - 최종 결전, 승패 판정
 *
 * ============================================================
 */
UCLASS()
class ETERNALDREAMS_API AEDGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AEDGameMode();

	// -------------------------------------------------------
	// Phase 제어 (외부 호출용)
	// -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void StartPhaseSequence();

	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void SkipToNextPhase();

	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	FGameplayTag GetCurrentPhase() const;

	/** 현재 몇 일차인지 (1~4). GameState 태그에서 파싱 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	int32 GetCurrentDay() const;

	/** 현재 밤인지. GameState 태그에서 파싱 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	bool IsNight() const;

	// -------------------------------------------------------
	// 사망 / 부활 / 승패
	// -------------------------------------------------------

	/**
	 * 플레이어 사망 진입점. (S1이 EDPlayerCharacter에서 HP 0 또는 SurvivalTime 0 감지 시 호출)
	 * Day 판정:
	 *   Day1~2 & RemainingRevives > 0 → SchedulePlayerRespawn
	 *   그 외 → EliminatePlayer
	 * 사망 직후 관전 상태로 전환한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|Death")
	void HandlePlayerDeath(AController* Victim, AController* Killer);

	/**
	 * 유저가 ZoneSelectWidget에서 구역을 선택하면 PlayerController의
	 * Server_RequestRespawn이 이 함수를 호출한다. DesiredZoneId 갱신 후 RestartPlayer.
	 */
	void HandleRespawnRequest(AController* Victim, int32 SelectedZoneId);

protected:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// -------------------------------------------------------
	// Starting System — 구역(Zone)별 스폰 위치 결정
	// -------------------------------------------------------

	/**
	 * 플레이어가 선택한 구역(DesiredZoneId)의 스폰 포인트 중
	 * 아직 사용되지 않은 하나를 랜덤으로 선택한다.
	 */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	// -------------------------------------------------------
	// Phase 설정 (에디터에서 지정)
	// -------------------------------------------------------

	/** Phase 순서 (Day1.Day → Day1.Night → Day2.Day → ...) */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	TArray<FGameplayTag> PhaseSequence;

	/** Phase별 지속 시간 (초). PhaseSequence와 1:1 매칭. 없으면 DefaultPhaseDuration 사용 */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	TArray<float> PhaseDurations;

	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	float DefaultPhaseDuration = 90.f;

	// -------------------------------------------------------
	// Phase 진입 콜백 — 각 Phase에서 해야 할 일을 여기에 추가
	// -------------------------------------------------------

	/** Phase가 변경될 때 호출. 하위 로직 분기점 */
	virtual void OnPhaseStarted(int32 PhaseIndex, const FGameplayTag& PhaseTag);

	// --- 일차별 낮 진입 ---
	virtual void OnDay1_DayStarted();
	virtual void OnDay1_NightStarted();
	virtual void OnDay2_DayStarted();
	virtual void OnDay2_NightStarted();
	virtual void OnDay3_DayStarted();
	virtual void OnDay3_NightStarted();
	virtual void OnDay4_DayStarted();
	virtual void OnDay4_NightStarted();

	/** 모든 Phase 소진 → 매치 종료 */
	virtual void OnMatchFinished();

	// -------------------------------------------------------
	// 사망 / 부활 내부 단계
	// -------------------------------------------------------

	/** 관전 전환: 기존 Pawn 파괴 + Spectating 상태 + 살아있는 팀원으로 ViewTarget 전환 */
	virtual void EnterSpectator(AController* Victim);

	/** 부활 가능: 10초 관전 후 ZoneSelectWidget 오픈 RPC. RestartPlayer는 HandleRespawnRequest에서 */
	virtual void SchedulePlayerRespawn(AController* Victim);

	/** 영구 사망: bEliminated=true, 관전 유지. CheckTeamElimination 호출 */
	virtual void EliminatePlayer(AController* Victim);

	/** 팀 전원 Eliminated 체크 → 탈락팀 처리. 남은 팀 1개면 승리 확정 */
	virtual void CheckTeamElimination();

	/** 팀원 중 살아있는 컨트롤러 반환 (없으면 nullptr). 관전 ViewTarget 후보 */
	AController* FindLivingTeammate(AController* Victim) const;

	// -------------------------------------------------------
	// 금지구역 제어
	// -------------------------------------------------------

	/** ZoneID → RestrictedArea 매핑. BeginPlay에서 자동 수집 */
	UPROPERTY()
	TMap<int32, AEDRestrictedArea*> RestrictedAreaMap;

	/** 금지구역 활성화 순서 (BeginPlay에서 셔플). 마지막 원소가 최종 안전구역 */
	UPROPERTY(VisibleInstanceOnly, Category = "ED|Zone")
	TArray<int32> RestrictedZoneOrder;

	/** 특정 구역의 금지구역을 활성화 */
	void ActivateRestrictedZone(int32 ZoneID);

	/** 레벨에 배치된 RestrictedArea를 수집하고 활성화 순서를 셔플 */
	void InitRestrictedZones();

private:
	// -------------------------------------------------------
	// Phase 내부 상태
	// -------------------------------------------------------

	int32 CurrentPhaseIndex = INDEX_NONE;
	float PhaseTimer = 0.f;
	bool bPhaseSequenceActive = false;

	void AdvanceToPhase(int32 PhaseIndex);
	float GetPhaseDuration(int32 PhaseIndex) const;
	void SetPhase(FGameplayTag NewPhase);

	// -------------------------------------------------------
	// Starting System 내부
	// -------------------------------------------------------

	/** 레벨의 AEDTeamPlayerStart를 구역별로 수집하여 캐시. BeginPlay에서 호출 */
	void CacheZonePlayerStarts();

	/** ZoneId → 해당 구역의 PlayerStart 배열 */
	TMap<int32, TArray<AEDPlayerStart*>> ZonePlayerStartMap;

	/** 이미 배정된 스폰 포인트 (중복 스폰 방지). 10초 후 자동 해제 */
	TSet<AEDPlayerStart*> OccupiedPlayerStarts;

	/** PlayerStart 점유 해제 타이머 핸들 (포인트별) */
	TMap<AEDPlayerStart*, FTimerHandle> PlayerStartReleaseTimers;

	/** 점유 유지 시간(초). 리스폰 시 같은 지점 재사용 가능하게 해제 */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Spawn")
	float PlayerStartOccupyDuration = 10.f;

	/** 사망 후 ZoneSelectWidget이 열리기까지 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Death")
	float RespawnZoneSelectDelay = 10.f;

	/** 사망한 플레이어의 ZoneSelect 오픈 타이머 핸들 */
	TMap<TWeakObjectPtr<AController>, FTimerHandle> RespawnTimers;

	// -------------------------------------------------------
	// 매치 종료 → 로비 복귀
	// -------------------------------------------------------

	/** 결과 UI 표시 후 로비 트래블까지 대기 시간(초) */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Match")
	float MatchEndDelay = 10.f;

	/** 매치 종료 후 복귀할 로비 맵 경로 */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Match")
	FString LobbyMapPath = TEXT("/Game/ED/Levels/L_Lobby");

	/** OnMatchFinished 중복 호출 방지 (CheckTeamElimination ↔ SkipToNextPhase 양쪽에서 호출 가능) */
	bool bMatchFinished = false;

	/** 매치 종료 → 로비 ServerTravel 타이머 */
	FTimerHandle MatchEndTravelHandle;

	/**
	 * [IOCP 전용] 전원 접속 시 Phase 시작.
	 * 현재는 BeginPlay에서 바로 시작하므로 비활성.
	 * 향후 IOCP 로비 연결 시, BeginPlay의 StartPhaseSequence 호출을 제거하고
	 * PostLogin에서 이 함수로 전원 접속 감지 후 시작하도록 전환.
	 */
	void TryStartPhaseSequence();
};
