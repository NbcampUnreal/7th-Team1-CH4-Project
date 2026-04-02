// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDPlayerController_Temp.generated.h"

/**
 * AEDPlayerController_Temp
 *
 * 로비 → 게임맵 전환 테스트용 임시 PlayerController.
 * 클라이언트의 Start 요청을 서버 GameMode에 전달한다.
 */
UCLASS()
class ETERNALDREAMS_API AEDPlayerController_Temp : public APlayerController
{
	GENERATED_BODY()

public:
	/** 클라이언트에서 호출 → 서버에서 실행. 게임 시작 요청. */
	UFUNCTION(Server, Reliable)
	void Server_RequestStartGame();
};
