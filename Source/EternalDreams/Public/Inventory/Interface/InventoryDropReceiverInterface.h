#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/InventoryTypes.h"
#include "UObject/Interface.h"
#include "InventoryDropReceiverInterface.generated.h"

UINTERFACE(BlueprintType)
class ETERNALDREAMS_API UInventoryDropReceiverInterface : public UInterface
{
    GENERATED_BODY()
};

class ETERNALDREAMS_API IInventoryDropReceiverInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
    void HandleInventoryDropRequest(const FInventoryDropRequest& DropRequest);
};

