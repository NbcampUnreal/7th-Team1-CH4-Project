#include "Inventory/System/InventoryTransferService.h"

#include "Inventory/Component/InventoryComponent.h"

bool FInventoryTransferService::MoveOrSwap(UInventoryComponent* InventoryComponent, int32 FromSlotIndex, int32 ToSlotIndex)
{
    return false;
}

bool FInventoryTransferService::TransferAuto(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    return false;
}

bool FInventoryTransferService::TransferToSlot(UInventoryComponent* FromInventory, UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    return false;
}

