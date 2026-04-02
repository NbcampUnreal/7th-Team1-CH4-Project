#include "Inventory/System/InventoryTransferService.h"

#include "Inventory/Component/InventoryComponent.h"

namespace
{
int32 ClampMoveQuantity(int32 RequestedQuantity, int32 AvailableQuantity)
{
    return FMath::Max(0, FMath::Min(RequestedQuantity, AvailableQuantity));
}
}

bool FInventoryTransferService::MoveOrSwap(UInventoryComponent* InventoryComponent, int32 FromSlotIndex, int32 ToSlotIndex)
{
    if (!InventoryComponent || FromSlotIndex == ToSlotIndex)
    {
        return false;
    }

    if (!InventoryComponent->InventorySlots.IsValidIndex(FromSlotIndex) || !InventoryComponent->InventorySlots.IsValidIndex(ToSlotIndex))
    {
        return false;
    }

    FInventorySlotData& FromSlot = InventoryComponent->InventorySlots[FromSlotIndex];
    FInventorySlotData& ToSlot = InventoryComponent->InventorySlots[ToSlotIndex];
    if (FromSlot.IsEmpty())
    {
        return false;
    }

    Swap(FromSlot, ToSlot);
    return true;
}

bool FInventoryTransferService::TransferAuto(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    if (!FromInventory || !ToInventory || !FromInventory->InventorySlots.IsValidIndex(FromSlotIndex))
    {
        return false;
    }

    FInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        return false;
    }

    const int32 MoveQuantity = ClampMoveQuantity(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        return false;
    }

    const FPrimaryAssetId SourceItemId = SourceSlot.Item.ItemId;
    int32 Remaining = MoveQuantity;

    for (int32 Index = 0; Index < ToInventory->InventorySlots.Num() && Remaining > 0; ++Index)
    {
        if (FromInventory == ToInventory && Index == FromSlotIndex)
        {
            continue;
        }

        FInventorySlotData& DestinationSlot = ToInventory->InventorySlots[Index];
        if (!DestinationSlot.IsEmpty() && DestinationSlot.Item.ItemId == SourceItemId)
        {
            const int32 AddAmount = FMath::Min(Remaining, MAX_int32 - DestinationSlot.Item.Quantity);
            if (AddAmount > 0)
            {
                DestinationSlot.Item.Quantity += AddAmount;
                Remaining -= AddAmount;
            }
        }
    }

    if (Remaining > 0)
    {
        for (int32 Index = 0; Index < ToInventory->InventorySlots.Num() && Remaining > 0; ++Index)
        {
            if (FromInventory == ToInventory && Index == FromSlotIndex)
            {
                continue;
            }

            FInventorySlotData& DestinationSlot = ToInventory->InventorySlots[Index];
            if (DestinationSlot.IsEmpty())
            {
                DestinationSlot.Item.ItemId = SourceItemId;
                DestinationSlot.Item.Quantity = Remaining;
                Remaining = 0;
                break;
            }
        }
    }

    const int32 MovedQuantity = MoveQuantity - Remaining;
    if (MovedQuantity <= 0)
    {
        return false;
    }

    SourceSlot.Item.Quantity -= MovedQuantity;
    if (SourceSlot.Item.Quantity <= 0)
    {
        SourceSlot.Item = FInventoryItemHandle();
    }

    return true;
}

bool FInventoryTransferService::TransferToSlot(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    if (!FromInventory || !ToInventory || (FromSlotIndex == ToSlotIndex && FromInventory == ToInventory))
    {
        return false;
    }

    if (!FromInventory->InventorySlots.IsValidIndex(FromSlotIndex) || !ToInventory->InventorySlots.IsValidIndex(ToSlotIndex))
    {
        return false;
    }

    FInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    FInventorySlotData& DestinationSlot = ToInventory->InventorySlots[ToSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        return false;
    }

    const int32 MoveQuantity = ClampMoveQuantity(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        return false;
    }

    if (DestinationSlot.IsEmpty())
    {
        DestinationSlot.Item.ItemId = SourceSlot.Item.ItemId;
        DestinationSlot.Item.Quantity = MoveQuantity;

        SourceSlot.Item.Quantity -= MoveQuantity;
        if (SourceSlot.Item.Quantity <= 0)
        {
            SourceSlot.Item = FInventoryItemHandle();
        }

        return true;
    }

    if (DestinationSlot.Item.ItemId == SourceSlot.Item.ItemId)
    {
        const int32 AddAmount = FMath::Min(MoveQuantity, MAX_int32 - DestinationSlot.Item.Quantity);
        if (AddAmount <= 0)
        {
            return false;
        }

        DestinationSlot.Item.Quantity += AddAmount;
        SourceSlot.Item.Quantity -= AddAmount;
        if (SourceSlot.Item.Quantity <= 0)
        {
            SourceSlot.Item = FInventoryItemHandle();
        }

        return true;
    }

    if (MoveQuantity != SourceSlot.Item.Quantity)
    {
        return false;
    }

    Swap(SourceSlot, DestinationSlot);
    return true;
}
