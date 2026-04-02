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

	/** 서버에서 호출. 모든 플레이어를 GameMap으로 이동시킨다. */
	UFUNCTION(BlueprintCallable, Category = "ED|GameMode")
	void StartGame();
};
