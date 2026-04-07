// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDLobbyPlayerController.generated.h"

/**
 * Lobby-only PlayerController.
 * Clients only report Ready state to the dedicated server.
 */
UCLASS()
class ETERNALDREAMS_API AEDLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable)
	void Server_SetReady();

	/** 팀 변경 요청 (클라이언트 → 서버) */
	UFUNCTION(Server, Reliable)
	void Server_ChangeTeam(int32 NewTeamId);
};
