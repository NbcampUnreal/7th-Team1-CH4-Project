#pragma once

#include "CoreMinimal.h"
#include "Item/Core/ItemTypes.h"
#include "Inventory/Core/InventoryTypes.h"

class UInventoryComponent;
class UInventoryItemDataAsset;
class AActor;

class FInventoryValidationService
{
public:
    static bool IsSlotIndexValid(const UInventoryComponent* InventoryComponent, int32 SlotIndex);
    static bool CanEquipToSlot(const UInventoryItemDataAsset* ItemData, EEquippableType TargetSlot);
    static bool CanConsumeItem(const UInventoryItemDataAsset* ItemData, const AActor* OwnerActor, EInventoryActionFailure* OutFailure = nullptr);
};
