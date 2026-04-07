#pragma once

#include "CoreMinimal.h"
#include "Item/Core/EDItemTypes.h"
#include "Inventory/Core/EDInventoryTypes.h"

class UEDInventoryComponent;
class UEDInventoryItemDataAsset;
class AActor;

class FEDInventoryValidationService
{
public:
    static bool IsSlotIndexValid(const UEDInventoryComponent* InventoryComponent, int32 SlotIndex);
    static bool CanEquipToSlot(const UEDInventoryItemDataAsset* ItemData, EEDEquippableType TargetSlot);
    static bool CanConsumeItem(const UEDInventoryItemDataAsset* ItemData, const AActor* OwnerActor, EEDInventoryActionFailure* OutFailure = nullptr);
};
