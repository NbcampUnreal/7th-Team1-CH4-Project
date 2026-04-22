// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/Types/EDPlayerTypes.h"
#include "GameFramework/PlayerState.h"
#include "EDPlayerState.generated.h"

enum class EPlayerNameType : uint8;

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

	void SetDisplayNickname(const FString& InDisplayNickname);
	const FString& GetDisplayNickname() const { return DisplayNickname; }

	// -------------------------------------------------------
	// 팀 / 준비 상태
	// -------------------------------------------------------

	/** 팀 ID (EDTeam::None = 미배정, 10 = TeamA, 11 = TeamB, 12 = TeamC) */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	int32 TeamId = EDTeam::None;

	/** 로비에서 입력한 플레이어 표시 이름 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Player")
	FString DisplayNickname;

	/** 준비 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	bool bReady = false;

	/** 희망 스폰 구역 ID (1~4). UI에서 선택 후 설정 */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	int32 DesiredZoneId = 0;
	
	//현석 : 플레이어 스킨 선택 (디폴트는 드루이드) UI에서 선택 후 설정
	UPROPERTY(BlueprintReadWrite, Category = "ED|Player", Replicated)
	EPlayerNameType PlayerSkinName=EPlayerNameType::Druid;
	
	UFUNCTION(BlueprintCallable,Server, Reliable)
	void SetPlayerSkinName(EPlayerNameType InPlayerSkinName);
	// -------------------------------------------------------
	// 사망 / 부활 / 전적
	// -------------------------------------------------------

	/** 현재 사망 상태 (부활 대기 또는 영구사망 포함) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Death")
	bool bIsDead = false;

	/** 영구 사망 여부. true면 더 이상 부활 불가 (게임 탈락) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Death")
	bool bEliminated = false;

	/** 남은 부활 횟수. Day1~2에서만 소모. 기본 2회 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Death")
	int32 RemainingRevives = 2;

	/** 킬 수 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Score")
	int32 Kills = 0;

	/** 데스 수 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Score")
	int32 Deaths = 0;
};
