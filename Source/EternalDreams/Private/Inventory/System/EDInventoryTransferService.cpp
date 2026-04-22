#include "Inventory/System/EDInventoryTransferService.h"

#include "Core/EDAssetManager.h"
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

    UEDAssetManager& AM = UEDAssetManager::Get(); 
    if (UEDInventoryItemDataAsset* CachedAsset = AM.GetPrimaryAsset<UEDInventoryItemDataAsset>(ItemId))
    {
        return CachedAsset;
    }
    return AM.LoadPrimaryAssetSync<UEDInventoryItemDataAsset>(ItemId);
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

void ShuffleIndices_Transfer(TArray<int32>& Indices, FRandomStream& RandomStream)
{
    for (int32 Index = Indices.Num() - 1; Index > 0; --Index)
    {
        const int32 SwapIndex = RandomStream.RandRange(0, Index);
        if (SwapIndex != Index)
        {
            Indices.Swap(Index, SwapIndex);
        }
    }
}

void BuildDestinationOrder_Transfer(
    const UEDInventoryComponent* FromInventory,
    const UEDInventoryComponent* ToInventory,
    int32 FromSlotIndex,
    const FPrimaryAssetId& SourceItemId,
    EEDInventoryStackPolicy StackPolicy,
    FRandomStream* InOutRandomStream,
    TArray<int32>& OutOrderedIndices)
{
    OutOrderedIndices.Reset();
    if (!ToInventory)
    {
        return;
    }

    TArray<int32> StackIndices;
    TArray<int32> EmptyIndices;

    for (int32 Index = 0; Index < ToInventory->InventorySlots.Num(); ++Index)
    {
        if (FromInventory == ToInventory && Index == FromSlotIndex)
        {
            continue;
        }

        const FEDInventorySlotData& DestinationSlot = ToInventory->InventorySlots[Index];
        if (DestinationSlot.IsEmpty())
        {
            EmptyIndices.Add(Index);
        }
        else if (DestinationSlot.Item.ItemId == SourceItemId)
        {
            StackIndices.Add(Index);
        }
    }

    switch (StackPolicy)
    {
    case EEDInventoryStackPolicy::SplitIfPossible:
        OutOrderedIndices.Append(EmptyIndices);
        OutOrderedIndices.Append(StackIndices);
        break;

    case EEDInventoryStackPolicy::UnionAsPossible:
        OutOrderedIndices.Append(StackIndices);
        OutOrderedIndices.Append(EmptyIndices);
        break;

    case EEDInventoryStackPolicy::Randomize:
    default:
        OutOrderedIndices.Append(StackIndices);
        OutOrderedIndices.Append(EmptyIndices);
        if (OutOrderedIndices.Num() > 1)
        {
            FRandomStream LocalRandom;
            if (!InOutRandomStream)
            {
                LocalRandom.GenerateNewSeed();
                ShuffleIndices_Transfer(OutOrderedIndices, LocalRandom);
            }
            else
            {
                ShuffleIndices_Transfer(OutOrderedIndices, *InOutRandomStream);
            }
        }
        break;
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
    return TransferAuto(
        FromInventory,
        ToInventory,
        FromSlotIndex,
        Quantity,
        OutFailure,
        EEDInventoryStackPolicy::UnionAsPossible,
        nullptr);
}

bool FEDInventoryTransferService::TransferAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure* OutFailure, EEDInventoryStackPolicy StackPolicy, FRandomStream* InOutRandomStream)
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

    TArray<int32> OrderedIndices;
    BuildDestinationOrder_Transfer(FromInventory, ToInventory, FromSlotIndex, SourceItemId, StackPolicy, InOutRandomStream, OrderedIndices);

    for (int32 DestinationIndex : OrderedIndices)
    {
        if (Remaining <= 0)
        {
            break;
        }

        FEDInventorySlotData& DestinationSlot = ToInventory->InventorySlots[DestinationIndex];
        if (DestinationSlot.IsEmpty())
        {
            const int32 AddAmount = FMath::Min(Remaining, MaxStack);
            if (AddAmount > 0)
            {
                DestinationSlot.Item.ItemId = SourceItemId;
                DestinationSlot.Item.Quantity = AddAmount;
                Remaining -= AddAmount;
            }
            continue;
        }

        if (DestinationSlot.Item.ItemId == SourceItemId)
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
