// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EDPlayerState.generated.h"

/** 팀 관련 상수. 몬스터 AI가 FGenericTeamId(1)을 사용하므로 플레이어 팀은 10번대로 배정 */
namespace EDTeam
{
	constexpr int32 None  = -1;  // 미배정
	constexpr int32 TeamA = 10;
	constexpr int32 TeamB = 11;
	constexpr int32 TeamC = 12;

	constexpr int32 PlayerTeamCount = 3;

	/** 팀 ID 배열 (인덱스 0~2 → TeamA~C) */
	constexpr int32 PlayerTeams[PlayerTeamCount] = { TeamA, TeamB, TeamC };

	/** 팀 표시 이름 */
	inline const TCHAR* GetTeamName(int32 TeamId)
	{
		switch (TeamId)
		{
		case TeamA: return TEXT("Team A");
		case TeamB: return TEXT("Team B");
		case TeamC: return TEXT("Team C");
		default:    return TEXT("None");
		}
	}

	/** 플레이어 팀인지 확인 */
	inline bool IsPlayerTeam(int32 TeamId)
	{
		return TeamId == TeamA || TeamId == TeamB || TeamId == TeamC;
	}
}

UCLASS()
class ETERNALDREAMS_API AEDPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AEDPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	// -------------------------------------------------------
	// 팀 / 준비 상태
	// -------------------------------------------------------

	/** 팀 ID (EDTeam::None = 미배정, 10 = TeamA, 11 = TeamB, 12 = TeamC) */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	int32 TeamId = EDTeam::None;

	/** 준비 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	bool bReady = false;

	/** 희망 스폰 구역 ID (1~4). UI에서 선택 후 설정 */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	int32 DesiredZoneId = 0;
};

