#include "Inventory/System/InventoryEquipmentService.h"

#include "Inventory/Component/InventoryComponent.h"

bool FInventoryEquipmentService::EquipFromSlot(UInventoryComponent* InventoryComponent, int32 FromSlotIndex, EEquippableType TargetSlotType)
{
    return false;
}

bool FInventoryEquipmentService::UnequipTopArmor(UInventoryComponent* InventoryComponent)
{
    return false;
}

bool FInventoryEquipmentService::UnequipBottomArmor(UInventoryComponent* InventoryComponent)
{
    return false;
}

bool FInventoryEquipmentService::EnsureDefaultWeapon(UInventoryComponent* InventoryComponent)
{
    return false;
}

