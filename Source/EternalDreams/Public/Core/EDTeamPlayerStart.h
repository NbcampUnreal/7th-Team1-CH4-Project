// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "EDTeamPlayerStart.generated.h"

/**
 * AEDTeamPlayerStart
 *
 * 팀별 스폰 위치를 지정하는 PlayerStart.
 * 레벨에 배치한 뒤 TeamId를 설정하면,
 * EDGameMode::ChoosePlayerStart에서 플레이어의 TeamId와 매칭하여 스폰 위치를 결정한다.
 *
 * 사용법:
 *   1. 맵에 AEDTeamPlayerStart를 팀별로 배치
 *   2. TeamId를 EDTeam::TeamA(10), TeamB(11), TeamC(12) 등으로 설정
 *   3. 같은 팀에 여러 개 배치하면 그 중 랜덤 선택
 */
UCLASS()
class ETERNALDREAMS_API AEDTeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	/** 이 스폰 포인트가 속한 팀 ID (EDTeam::TeamA=10, TeamB=11, TeamC=12) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Spawn")
	int32 TeamId = -1;
};
