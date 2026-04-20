#pragma once

#include "CoreMinimal.h"
#include "Item/Core/EDItemTypes.h"

class UEDInventoryComponent;
enum class EEDSkillSlotType : uint8;

class FEDInventoryEquipmentService
{
public:
    static bool EquipFromSlot(UEDInventoryComponent* InventoryComponent, int32 FromSlotIndex, EEDEquippableType TargetSlotType);
    static bool UnequipTopArmor(UEDInventoryComponent* InventoryComponent);
    static bool UnequipBottomArmor(UEDInventoryComponent* InventoryComponent);
    static bool EquipSkillFromSlot(UEDInventoryComponent* InventoryComponent, int32 FromSlotIndex, EEDSkillSlotType TargetSkillSlotType);
    static bool UnequipSkillSlot(UEDInventoryComponent* InventoryComponent, EEDSkillSlotType SkillSlotType);
    static bool EnsureDefaultWeapon(UEDInventoryComponent* InventoryComponent);
};
