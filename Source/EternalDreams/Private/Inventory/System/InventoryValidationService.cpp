#include "Inventory/System/InventoryValidationService.h"

#include "Inventory/Component/InventoryComponent.h"
#include "Item/Data/InventoryItemDataAsset.h"

bool FInventoryValidationService::IsSlotIndexValid(const UInventoryComponent* InventoryComponent, int32 SlotIndex)
{
    return InventoryComponent && InventoryComponent->InventorySlots.IsValidIndex(SlotIndex);
}

bool FInventoryValidationService::CanEquipToSlot(const UInventoryItemDataAsset* ItemData, EEquippableType TargetSlot)
{
    if (!ItemData)
    {
        return false;
    }

    if (ItemData->ItemType != EInventoryItemType::Equippable)
    {
        return false;
    }

    return ItemData->EquippableType == TargetSlot;
}

bool FInventoryValidationService::CanConsumeItem(const UInventoryItemDataAsset* ItemData, const AActor* OwnerActor)
{
    return ItemData && ItemData->ItemType == EInventoryItemType::Consumable;
}
