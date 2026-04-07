#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EDInventoryProviderInterface.generated.h"

class UEDInventoryComponent;

UINTERFACE(BlueprintType)
class ETERNALDREAMS_API UEDInventoryProviderInterface : public UInterface
{
    GENERATED_BODY()
};

class ETERNALDREAMS_API IEDInventoryProviderInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
    UEDInventoryComponent* GetInventoryComponent() const;
};
