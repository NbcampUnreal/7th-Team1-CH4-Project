#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryBlueprintLibrary.generated.h"

class AActor;
class UInventoryComponent;

UCLASS()
class ETERNALDREAMS_API UInventoryBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    static UInventoryComponent* GetInventoryComponentFromActor(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    static bool IsValidInventorySlotIndex(const UInventoryComponent* InventoryComponent, int32 SlotIndex);
};

