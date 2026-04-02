// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EDPlayerState.generated.h"

UCLASS()
class ETERNALDREAMS_API AEDPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AEDPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
