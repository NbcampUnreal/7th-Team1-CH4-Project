// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDPlayerController_Temp.generated.h"

/**
 * AEDPlayerController_Temp
 *
 * InGame(게임맵) 테스트용 임시 PlayerController.
 * 페이즈 관련 테스트 RPC만 포함. 로비 로직은 AEDLobbyPlayerController��� 이동.
 */
UCLASS()
class ETERNALDREAMS_API AEDPlayerController_Temp : public APlayerController
{
	GENERATED_BODY()

public:
	/** 클라이언��에서 호출 → 서버에서 실행. 페이즈 시퀀스 시작 요청. (테스트용) */
	UFUNCTION(Server, Reliable)
	void Server_RequestStartPhaseSequence();

	/** 클라이언트에서 호출 → 서버에서 실행. 다음 페이즈 스킵 요청. (테스트용) */
	UFUNCTION(Server, Reliable)
	void Server_RequestSkipPhase();
};
