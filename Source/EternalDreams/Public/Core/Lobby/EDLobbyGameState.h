// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "EDLobbyGameState.generated.h"

/**
 * AEDLobbyGameState
 *
 * 로비 전용 GameState.
 * 팀별 인원 수 등 로비 UI에서 필요한 공용 데이터를 리플리케이트한다.
 */
UCLASS()
class ETERNALDREAMS_API AEDLobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	AEDLobbyGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** PlayerArray를 순회하여 팀 인원 갱신 (서버에서 호출) */
	void UpdateTeamCounts();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Lobby")
	int32 TeamACount = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "ED|Lobby")
	int32 TeamBCount = 0;
};
