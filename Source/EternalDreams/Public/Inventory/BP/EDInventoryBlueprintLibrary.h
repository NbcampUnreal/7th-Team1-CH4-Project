#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EDInventoryBlueprintLibrary.generated.h"

class AActor;
class UEDInventoryComponent;

UCLASS()
class ETERNALDREAMS_API UEDInventoryBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    static UEDInventoryComponent* GetInventoryComponentFromActor(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    static bool IsValidInventorySlotIndex(const UEDInventoryComponent* InventoryComponent, int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "Inventory|Failure")
    static bool IsInventoryActionSuccess(EEDInventoryActionFailure Failure);

    UFUNCTION(BlueprintPure, Category = "Inventory|Failure")
    static FText GetInventoryActionFailureText(EEDInventoryActionFailure Failure);
};
