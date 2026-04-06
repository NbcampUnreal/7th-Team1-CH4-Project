#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/InventoryTypes.h"

class UInventoryComponent;

class FInventoryTransferService
{
public:
    static bool MoveOrSwap(UInventoryComponent* InventoryComponent, int32 FromSlotIndex, int32 ToSlotIndex);
    static bool TransferAuto(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EInventoryActionFailure* OutFailure = nullptr);
    static bool TransferToSlot(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EInventoryActionFailure* OutFailure = nullptr);
};
