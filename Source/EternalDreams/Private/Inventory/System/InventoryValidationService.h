#pragma once

#include "CoreMinimal.h"
#include "Item/Core/ItemTypes.h"

class UInventoryComponent;
class UInventoryItemDataAsset;

class FInventoryValidationService
{
public:
    static bool IsSlotIndexValid(const UInventoryComponent* InventoryComponent, int32 SlotIndex);
    static bool CanEquipToSlot(const UInventoryItemDataAsset* ItemData, EEquippableType TargetSlot);
    static bool CanConsumeItem(const UInventoryItemDataAsset* ItemData, const AActor* OwnerActor);
};
