#include "Inventory/System/EDInventoryEquipmentService.h"

#include "Engine/AssetManager.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/System/EDInventoryValidationService.h"
#include "Item/Data/EDInventoryItemDataAsset.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveItemData_Equipment(const FPrimaryAssetId& ItemId)
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

    return Cast<UEDInventoryItemDataAsset>(ItemObject);
}

FEDEquipmentSlotData* GetEquipmentSlot_Equipment(UEDInventoryComponent* InventoryComponent, EEDEquippableType SlotType)
{
    if (!InventoryComponent)
    {
        return nullptr;
    }

    switch (SlotType)
    {
    case EEDEquippableType::Weapon:
        return &InventoryComponent->WeaponSlot;
    case EEDEquippableType::TopArmor:
        return &InventoryComponent->TopArmorSlot;
    case EEDEquippableType::BottomArmor:
        return &InventoryComponent->BottomArmorSlot;
    default:
        return nullptr;
    }
}

bool TryUnequipToInventoryOrDrop(UEDInventoryComponent* InventoryComponent, FEDEquipmentSlotData& EquipmentSlot)
{
    if (!InventoryComponent || !EquipmentSlot.EquippedItem.IsValid())
    {
        return false;
    }

    for (FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Slot.IsEmpty())
        {
            Slot.Item = EquipmentSlot.EquippedItem;
            EquipmentSlot.EquippedItem = FEDInventoryItemHandle();
            return true;
        }
    }

    FEDInventoryDropRequest DropRequest;
    DropRequest.Item = EquipmentSlot.EquippedItem;
    DropRequest.SourceOwner = InventoryComponent->GetOwner();
    DropRequest.Reason = EEDInventoryDropReason::UnequipNoSpace;
    InventoryComponent->OnInventoryDropRequested.Broadcast(DropRequest);

    EquipmentSlot.EquippedItem = FEDInventoryItemHandle();
    return true;
}
}

bool FEDInventoryEquipmentService::EquipFromSlot(UEDInventoryComponent* InventoryComponent, int32 FromSlotIndex, EEDEquippableType TargetSlotType)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    if (!InventoryComponent->InventorySlots.IsValidIndex(FromSlotIndex))
    {
        return false;
    }

    FEDInventorySlotData& SourceSlot = InventoryComponent->InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        return false;
    }

    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Equipment(SourceSlot.Item.ItemId);
    if (!FEDInventoryValidationService::CanEquipToSlot(ItemData, TargetSlotType))
    {
        return false;
    }

    FEDEquipmentSlotData* EquipmentSlot = GetEquipmentSlot_Equipment(InventoryComponent, TargetSlotType);
    if (!EquipmentSlot)
    {
        return false;
    }

    if (EquipmentSlot->EquippedItem.IsValid())
    {
        const FEDInventoryItemHandle PreviousEquippedItem = EquipmentSlot->EquippedItem;
        EquipmentSlot->EquippedItem = SourceSlot.Item;
        SourceSlot.Item = PreviousEquippedItem;
        return true;
    }

    EquipmentSlot->EquippedItem = SourceSlot.Item;
    SourceSlot.Item = FEDInventoryItemHandle();
    return true;
}

bool FEDInventoryEquipmentService::UnequipTopArmor(UEDInventoryComponent* InventoryComponent)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    return TryUnequipToInventoryOrDrop(InventoryComponent, InventoryComponent->TopArmorSlot);
}

bool FEDInventoryEquipmentService::UnequipBottomArmor(UEDInventoryComponent* InventoryComponent)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    return TryUnequipToInventoryOrDrop(InventoryComponent, InventoryComponent->BottomArmorSlot);
}

bool FEDInventoryEquipmentService::EnsureDefaultWeapon(UEDInventoryComponent* InventoryComponent)
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
