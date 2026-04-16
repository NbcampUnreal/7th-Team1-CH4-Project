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

UENUM(BlueprintType)
enum class EEDUIMessageType : uint8
{
	Info UMETA(DisplayName = "Info"),
	Success UMETA(DisplayName = "Success"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error")
};
