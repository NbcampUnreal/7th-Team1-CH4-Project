// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDLobbyPlayerEntry.generated.h"

class UTextBlock;

/**
 * UEDLobbyPlayerEntry
 *
 * 로비 플레이어 리스트의 개별 항목 위젯.
 * BP에서 상속받아 레이아웃만 배치.
 */
UCLASS(Abstract)
class ETERNALDREAMS_API UEDLobbyPlayerEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 플레이어 정보를 세팅 */
	UFUNCTION(BlueprintCallable, Category = "UI|Lobby")
	void SetPlayerInfo(const FString& InNickname, int32 InTeamId, bool bInReady);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NicknameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyText;
};
