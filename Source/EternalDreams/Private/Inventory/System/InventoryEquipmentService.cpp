#include "Inventory/System/InventoryEquipmentService.h"

#include "Engine/AssetManager.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/System/InventoryValidationService.h"
#include "Item/Data/InventoryItemDataAsset.h"

namespace
{
const UInventoryItemDataAsset* ResolveItemData(const FPrimaryAssetId& ItemId)
{
    if (!ItemId.IsValid())
    {
        return nullptr;
    }

    UObject* ItemObject = UAssetManager::Get().GetPrimaryAssetObject(ItemId);
    if (!ItemObject)
    {
        const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
        if (AssetPath.IsValid())
        {
            ItemObject = AssetPath.TryLoad();
        }
    }

    return Cast<UInventoryItemDataAsset>(ItemObject);
}

FEquipmentSlotData* GetEquipmentSlot(UInventoryComponent* InventoryComponent, EEquippableType SlotType)
{
    if (!InventoryComponent)
    {
        return nullptr;
    }

    switch (SlotType)
    {
    case EEquippableType::Weapon:
        return &InventoryComponent->WeaponSlot;
    case EEquippableType::TopArmor:
        return &InventoryComponent->TopArmorSlot;
    case EEquippableType::BottomArmor:
        return &InventoryComponent->BottomArmorSlot;
    default:
        return nullptr;
    }
}

bool TryUnequipToInventoryOrDrop(UInventoryComponent* InventoryComponent, FEquipmentSlotData& EquipmentSlot)
{
    if (!InventoryComponent || !EquipmentSlot.EquippedItem.IsValid())
    {
        return false;
    }

    for (FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Slot.IsEmpty())
        {
            Slot.Item = EquipmentSlot.EquippedItem;
            EquipmentSlot.EquippedItem = FInventoryItemHandle();
            return true;
        }
    }

    FInventoryDropRequest DropRequest;
    DropRequest.Item = EquipmentSlot.EquippedItem;
    DropRequest.SourceOwner = InventoryComponent->GetOwner();
    DropRequest.Reason = EInventoryDropReason::UnequipNoSpace;
    InventoryComponent->OnInventoryDropRequested.Broadcast(DropRequest);

    EquipmentSlot.EquippedItem = FInventoryItemHandle();
    return true;
}
}

bool FInventoryEquipmentService::EquipFromSlot(UInventoryComponent* InventoryComponent, int32 FromSlotIndex, EEquippableType TargetSlotType)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    if (!InventoryComponent->InventorySlots.IsValidIndex(FromSlotIndex))
    {
        return false;
    }

    FInventorySlotData& SourceSlot = InventoryComponent->InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        return false;
    }

    const UInventoryItemDataAsset* ItemData = ResolveItemData(SourceSlot.Item.ItemId);
    if (!FInventoryValidationService::CanEquipToSlot(ItemData, TargetSlotType))
    {
        return false;
    }

    FEquipmentSlotData* EquipmentSlot = GetEquipmentSlot(InventoryComponent, TargetSlotType);
    if (!EquipmentSlot)
    {
        return false;
    }

    if (EquipmentSlot->EquippedItem.IsValid())
    {
        const FInventoryItemHandle PreviousEquippedItem = EquipmentSlot->EquippedItem;
        EquipmentSlot->EquippedItem = SourceSlot.Item;
        SourceSlot.Item = PreviousEquippedItem;
        return true;
    }

    EquipmentSlot->EquippedItem = SourceSlot.Item;
    SourceSlot.Item = FInventoryItemHandle();
    return true;
}

bool FInventoryEquipmentService::UnequipTopArmor(UInventoryComponent* InventoryComponent)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    return TryUnequipToInventoryOrDrop(InventoryComponent, InventoryComponent->TopArmorSlot);
}

bool FInventoryEquipmentService::UnequipBottomArmor(UInventoryComponent* InventoryComponent)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    return TryUnequipToInventoryOrDrop(InventoryComponent, InventoryComponent->BottomArmorSlot);
}

bool FInventoryEquipmentService::EnsureDefaultWeapon(UInventoryComponent* InventoryComponent)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    if (InventoryComponent->WeaponSlot.EquippedItem.IsValid())
    {
        return true;
    }

    if (!InventoryComponent->DefaultWeaponItemId.IsValid())
    {
        return false;
    }

    InventoryComponent->WeaponSlot.EquippedItem.ItemId = InventoryComponent->DefaultWeaponItemId;
    InventoryComponent->WeaponSlot.EquippedItem.Quantity = 1;
    return true;
}
