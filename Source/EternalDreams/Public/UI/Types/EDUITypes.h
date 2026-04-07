#pragma once

#include "CoreMinimal.h"
#include "EDUITypes.generated.h"

UENUM(BlueprintType)
enum class EEDUILayer : uint8
{
	Game UMETA(DisplayName = "Game"),
	Menu UMETA(DisplayName = "Menu"),
	Modal UMETA(DisplayName = "Modal")
};
