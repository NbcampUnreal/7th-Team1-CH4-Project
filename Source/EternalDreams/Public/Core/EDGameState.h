// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "EDGameState.generated.h"


UCLASS()
class ETERNALDREAMS_API AEDGameState : public AGameState
{
	GENERATED_BODY()

public:
	AEDGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
