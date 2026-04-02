#pragma once

#include "CoreMinimal.h"
#include "Item/Core/ItemTypes.h"

class UInventoryComponent;

class FInventoryEquipmentService
{
public:
    static bool EquipFromSlot(UInventoryComponent* InventoryComponent, int32 FromSlotIndex, EEquippableType TargetSlotType);
    static bool UnequipTopArmor(UInventoryComponent* InventoryComponent);
    static bool UnequipBottomArmor(UInventoryComponent* InventoryComponent);
    static bool EnsureDefaultWeapon(UInventoryComponent* InventoryComponent);
};
