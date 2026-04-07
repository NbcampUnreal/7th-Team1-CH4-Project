#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "UObject/Interface.h"
#include "EDInventoryDropReceiverInterface.generated.h"

UINTERFACE(BlueprintType)
class ETERNALDREAMS_API UEDInventoryDropReceiverInterface : public UInterface
{
    GENERATED_BODY()
};

class ETERNALDREAMS_API IEDInventoryDropReceiverInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
    void HandleInventoryDropRequest(const FEDInventoryDropRequest& DropRequest);
};
