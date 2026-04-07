#include "Inventory/System/EDInventoryTransferService.h"

#include "Engine/AssetManager.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"

namespace
{
int32 ClampMoveQuantity_Transfer(int32 RequestedQuantity, int32 AvailableQuantity)
{
    return FMath::Max(0, FMath::Min(RequestedQuantity, AvailableQuantity));
}

const UEDInventoryItemDataAsset* ResolveItemData_Transfer(const FPrimaryAssetId& ItemId)
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

int32 GetItemMaxStack_Transfer(const FPrimaryAssetId& ItemId)
{
    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Transfer(ItemId);
    return ItemData ? FMath::Max(1, ItemData->MaxStack) : 1;
}

void SetFailure_Transfer(EEDInventoryActionFailure* OutFailure, EEDInventoryActionFailure Failure)
{
    if (OutFailure)
    {
        *OutFailure = Failure;
    }
}
}

bool FEDInventoryTransferService::MoveOrSwap(UEDInventoryComponent* InventoryComponent, int32 FromSlotIndex, int32 ToSlotIndex)
{
    if (!InventoryComponent || FromSlotIndex == ToSlotIndex)
    {
        return false;
    }

    if (!InventoryComponent->InventorySlots.IsValidIndex(FromSlotIndex) || !InventoryComponent->InventorySlots.IsValidIndex(ToSlotIndex))
    {
        return false;
    }

    FEDInventorySlotData& FromSlot = InventoryComponent->InventorySlots[FromSlotIndex];
    FEDInventorySlotData& ToSlot = InventoryComponent->InventorySlots[ToSlotIndex];
    if (FromSlot.IsEmpty())
    {
        return false;
    }

    Swap(FromSlot, ToSlot);
    return true;
}

bool FEDInventoryTransferService::TransferAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure* OutFailure)
{
    SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::None);

    if (!FromInventory || !ToInventory)
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::InvalidInventory);
        return false;
    }

    if (!FromInventory->InventorySlots.IsValidIndex(FromSlotIndex))
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::InvalidSlot);
        return false;
    }

    FEDInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::EmptySlot);
        return false;
    }

    const int32 MoveQuantity = ClampMoveQuantity_Transfer(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::InvalidQuantity);
        return false;
    }

    const FPrimaryAssetId SourceItemId = SourceSlot.Item.ItemId;
    const int32 MaxStack = GetItemMaxStack_Transfer(SourceItemId);
    int32 Remaining = MoveQuantity;

    for (int32 Index = 0; Index < ToInventory->InventorySlots.Num() && Remaining > 0; ++Index)
    {
        if (FromInventory == ToInventory && Index == FromSlotIndex)
        {
            continue;
        }

        FEDInventorySlotData& DestinationSlot = ToInventory->InventorySlots[Index];
        if (!DestinationSlot.IsEmpty() && DestinationSlot.Item.ItemId == SourceItemId)
        {
            const int32 SpaceLeft = FMath::Max(0, MaxStack - DestinationSlot.Item.Quantity);
            const int32 AddAmount = FMath::Min(Remaining, SpaceLeft);
            if (AddAmount > 0)
            {
                DestinationSlot.Item.Quantity += AddAmount;
                Remaining -= AddAmount;
            }
        }
    }

    for (int32 Index = 0; Index < ToInventory->InventorySlots.Num() && Remaining > 0; ++Index)
    {
        if (FromInventory == ToInventory && Index == FromSlotIndex)
        {
            continue;
        }

        FEDInventorySlotData& DestinationSlot = ToInventory->InventorySlots[Index];
        if (DestinationSlot.IsEmpty())
        {
            const int32 AddAmount = FMath::Min(Remaining, MaxStack);
            DestinationSlot.Item.ItemId = SourceItemId;
            DestinationSlot.Item.Quantity = AddAmount;
            Remaining -= AddAmount;
        }
    }

    const int32 MovedQuantity = MoveQuantity - Remaining;
    if (MovedQuantity <= 0)
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::NoSpace);
        return false;
    }

    SourceSlot.Item.Quantity -= MovedQuantity;
    if (SourceSlot.Item.Quantity <= 0)
    {
        SourceSlot.Item = FEDInventoryItemHandle();
    }

    return true;
}

bool FEDInventoryTransferService::TransferToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure* OutFailure)
{
    SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::None);

    if (!FromInventory || !ToInventory)
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::InvalidInventory);
        return false;
    }

    if (FromSlotIndex == ToSlotIndex && FromInventory == ToInventory)
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::SlotConflict);
        return false;
    }

    if (!FromInventory->InventorySlots.IsValidIndex(FromSlotIndex) || !ToInventory->InventorySlots.IsValidIndex(ToSlotIndex))
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::InvalidSlot);
        return false;
    }

    FEDInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    FEDInventorySlotData& DestinationSlot = ToInventory->InventorySlots[ToSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::EmptySlot);
        return false;
    }

    const int32 MoveQuantity = ClampMoveQuantity_Transfer(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::InvalidQuantity);
        return false;
    }

    const int32 MaxStack = GetItemMaxStack_Transfer(SourceSlot.Item.ItemId);

    if (DestinationSlot.IsEmpty())
    {
        const int32 AddAmount = FMath::Min(MoveQuantity, MaxStack);
        if (AddAmount <= 0)
        {
            SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::StackLimit);
            return false;
        }

        DestinationSlot.Item.ItemId = SourceSlot.Item.ItemId;
        DestinationSlot.Item.Quantity = AddAmount;

        SourceSlot.Item.Quantity -= AddAmount;
        if (SourceSlot.Item.Quantity <= 0)
        {
            SourceSlot.Item = FEDInventoryItemHandle();
        }

        return true;
    }

    if (DestinationSlot.Item.ItemId == SourceSlot.Item.ItemId)
    {
        const int32 SpaceLeft = FMath::Max(0, MaxStack - DestinationSlot.Item.Quantity);
        const int32 AddAmount = FMath::Min(MoveQuantity, SpaceLeft);
        if (AddAmount <= 0)
        {
            SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::StackLimit);
            return false;
        }

        DestinationSlot.Item.Quantity += AddAmount;
        SourceSlot.Item.Quantity -= AddAmount;
        if (SourceSlot.Item.Quantity <= 0)
        {
            SourceSlot.Item = FEDInventoryItemHandle();
        }

        return true;
    }

    if (MoveQuantity != SourceSlot.Item.Quantity)
    {
        SetFailure_Transfer(OutFailure, EEDInventoryActionFailure::SlotConflict);
        return false;
    }

    Swap(SourceSlot, DestinationSlot);
    return true;
}
