#pragma once

#include "CoreMinimal.h"
#include "Item/Core/EDItemTypes.h"

class UEDInventoryComponent;

class FEDInventoryEquipmentService
{
public:
    static bool EquipFromSlot(UEDInventoryComponent* InventoryComponent, int32 FromSlotIndex, EEDEquippableType TargetSlotType);
    static bool UnequipTopArmor(UEDInventoryComponent* InventoryComponent);
    static bool UnequipBottomArmor(UEDInventoryComponent* InventoryComponent);
    static bool EnsureDefaultWeapon(UEDInventoryComponent* InventoryComponent);
};
