// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "EDTeamPlayerStart.generated.h"

/**
 * AEDTeamPlayerStart
 *
 * 구역(Zone)별 스폰 위치를 지정하는 PlayerStart.
 * 레벨에 배치한 뒤 ZoneId를 설정하면,
 * EDGameMode::ChoosePlayerStart에서 플레이어가 선택한 구역의 스폰 포인트 중
 * 사용되지 않은 하나를 랜덤으로 선택하여 스폰한다.
 *
 * 사용법:
 *   1. 맵에 구역당 6개씩 AEDTeamPlayerStart를 배치
 *   2. ZoneId를 구역 번호(1~4)로 설정
 *   3. 같은 구역 내 겹치지 않게 랜덤 배정
 */
UCLASS()
class ETERNALDREAMS_API AEDTeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	/** 이 스폰 포인트가 속한 구역 ID (1~4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ED|Spawn")
	int32 ZoneId = 0;
};
