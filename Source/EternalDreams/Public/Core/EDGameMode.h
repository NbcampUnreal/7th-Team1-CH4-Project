// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EDGameMode.generated.h"

UCLASS()
class ETERNALDREAMS_API AEDGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AEDGameMode();

	/** BP_GameMode에서 설정할 게임맵 경로 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ED|GameMode")
	FString GameMapPath = TEXT("/Game/ED/Asset/IgnoredAsset/Test/GameMap");

	/** 서버에서 호출. 모든 플레이어를 GameMap으로 이동시킨다. */
	UFUNCTION(BlueprintCallable, Category = "ED|GameMode")
	void StartGame();
};
