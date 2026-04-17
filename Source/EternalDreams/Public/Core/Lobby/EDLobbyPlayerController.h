// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EDLobbyPlayerController.generated.h"

/**
 * Lobby-only PlayerController.
 * UI 전용 입력 모드, 로비 위젯 자동 생성.
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

	/** 스폰 구역 선택 요청 (클라이언트 → 서버) */
	UFUNCTION(Server, Reliable)
	void Server_SelectZone(int32 ZoneId);

protected:
	virtual void BeginPlay() override;

	/** 로비 위젯 클래스 (BP에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Lobby|UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

private:
	UFUNCTION()
	void HandleDirectZoneSelection(int32 ZoneId);

	UPROPERTY()
	TObjectPtr<UUserWidget> LobbyWidget;
};
