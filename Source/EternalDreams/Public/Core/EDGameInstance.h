// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EDGameInstance.generated.h"


UCLASS()
class ETERNALDREAMS_API UEDGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UEDGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;
};
