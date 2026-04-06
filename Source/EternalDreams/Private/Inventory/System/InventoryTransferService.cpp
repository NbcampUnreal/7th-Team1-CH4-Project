#include "Inventory/System/InventoryTransferService.h"

#include "Engine/AssetManager.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Item/Data/InventoryItemDataAsset.h"

namespace
{
int32 ClampMoveQuantity(int32 RequestedQuantity, int32 AvailableQuantity)
{
    return FMath::Max(0, FMath::Min(RequestedQuantity, AvailableQuantity));
}

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

int32 GetItemMaxStack(const FPrimaryAssetId& ItemId)
{
    const UInventoryItemDataAsset* ItemData = ResolveItemData(ItemId);
    return ItemData ? FMath::Max(1, ItemData->MaxStack) : 1;
}

void SetFailure(EInventoryActionFailure* OutFailure, EInventoryActionFailure Failure)
{
    if (OutFailure)
    {
        *OutFailure = Failure;
    }
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

bool FInventoryTransferService::TransferAuto(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EInventoryActionFailure* OutFailure)
{
    SetFailure(OutFailure, EInventoryActionFailure::None);

    if (!FromInventory || !ToInventory)
    {
        SetFailure(OutFailure, EInventoryActionFailure::InvalidInventory);
        return false;
    }

    if (!FromInventory->InventorySlots.IsValidIndex(FromSlotIndex))
    {
        SetFailure(OutFailure, EInventoryActionFailure::InvalidSlot);
        return false;
    }

    FInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        SetFailure(OutFailure, EInventoryActionFailure::EmptySlot);
        return false;
    }

    const int32 MoveQuantity = ClampMoveQuantity(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        SetFailure(OutFailure, EInventoryActionFailure::InvalidQuantity);
        return false;
    }

    const FPrimaryAssetId SourceItemId = SourceSlot.Item.ItemId;
    const int32 MaxStack = GetItemMaxStack(SourceItemId);
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

        FInventorySlotData& DestinationSlot = ToInventory->InventorySlots[Index];
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
        SetFailure(OutFailure, EInventoryActionFailure::NoSpace);
        return false;
    }

    SourceSlot.Item.Quantity -= MovedQuantity;
    if (SourceSlot.Item.Quantity <= 0)
    {
        SourceSlot.Item = FInventoryItemHandle();
    }

    return true;
}

bool FInventoryTransferService::TransferToSlot(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EInventoryActionFailure* OutFailure)
{
    SetFailure(OutFailure, EInventoryActionFailure::None);

    if (!FromInventory || !ToInventory)
    {
        SetFailure(OutFailure, EInventoryActionFailure::InvalidInventory);
        return false;
    }

    if (FromSlotIndex == ToSlotIndex && FromInventory == ToInventory)
    {
        SetFailure(OutFailure, EInventoryActionFailure::SlotConflict);
        return false;
    }

    if (!FromInventory->InventorySlots.IsValidIndex(FromSlotIndex) || !ToInventory->InventorySlots.IsValidIndex(ToSlotIndex))
    {
        SetFailure(OutFailure, EInventoryActionFailure::InvalidSlot);
        return false;
    }

    FInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    FInventorySlotData& DestinationSlot = ToInventory->InventorySlots[ToSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        SetFailure(OutFailure, EInventoryActionFailure::EmptySlot);
        return false;
    }

    const int32 MoveQuantity = ClampMoveQuantity(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        SetFailure(OutFailure, EInventoryActionFailure::InvalidQuantity);
        return false;
    }

    const int32 MaxStack = GetItemMaxStack(SourceSlot.Item.ItemId);

    if (DestinationSlot.IsEmpty())
    {
        const int32 AddAmount = FMath::Min(MoveQuantity, MaxStack);
        if (AddAmount <= 0)
        {
            SetFailure(OutFailure, EInventoryActionFailure::StackLimit);
            return false;
        }

        DestinationSlot.Item.ItemId = SourceSlot.Item.ItemId;
        DestinationSlot.Item.Quantity = AddAmount;

        SourceSlot.Item.Quantity -= AddAmount;
        if (SourceSlot.Item.Quantity <= 0)
        {
            SourceSlot.Item = FInventoryItemHandle();
        }

        return true;
    }

    if (DestinationSlot.Item.ItemId == SourceSlot.Item.ItemId)
    {
        const int32 SpaceLeft = FMath::Max(0, MaxStack - DestinationSlot.Item.Quantity);
        const int32 AddAmount = FMath::Min(MoveQuantity, SpaceLeft);
        if (AddAmount <= 0)
        {
            SetFailure(OutFailure, EInventoryActionFailure::StackLimit);
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
        SetFailure(OutFailure, EInventoryActionFailure::SlotConflict);
        return false;
    }

    Swap(SourceSlot, DestinationSlot);
    return true;
}
