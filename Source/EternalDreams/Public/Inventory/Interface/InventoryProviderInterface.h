#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InventoryProviderInterface.generated.h"

class UInventoryComponent;

UINTERFACE(BlueprintType)
class ETERNALDREAMS_API UInventoryProviderInterface : public UInterface
{
    GENERATED_BODY()
};

class ETERNALDREAMS_API IInventoryProviderInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
    UInventoryComponent* GetInventoryComponent() const;
};

