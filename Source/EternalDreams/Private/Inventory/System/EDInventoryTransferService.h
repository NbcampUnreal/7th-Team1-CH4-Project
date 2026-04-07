#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"

class UEDInventoryComponent;

class FEDInventoryTransferService
{
public:
    static bool MoveOrSwap(UEDInventoryComponent* InventoryComponent, int32 FromSlotIndex, int32 ToSlotIndex);
    static bool TransferAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure* OutFailure = nullptr);
    static bool TransferToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure* OutFailure = nullptr);
};
