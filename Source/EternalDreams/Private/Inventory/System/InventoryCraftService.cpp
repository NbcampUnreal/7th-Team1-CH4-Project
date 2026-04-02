#include "Inventory/System/InventoryCraftService.h"

#include "Inventory/Component/InventoryComponent.h"
#include "Item/Data/ItemDataRows.h"

namespace
{
int32 CountItemInInventory(const UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    int32 Count = 0;
    for (const FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (!Slot.IsEmpty() && Slot.Item.ItemId == ItemId)
        {
            Count += Slot.Item.Quantity;
        }
    }

    return Count;
}

bool CanStoreResultItem(const UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    for (const FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Slot.IsEmpty() || Slot.Item.ItemId == ItemId)
        {
            return true;
        }
    }

    return false;
}
}

bool FInventoryCraftService::TryCraftByRecipeId(UInventoryComponent* InventoryComponent, FName RecipeId)
{
    if (!InventoryComponent || !InventoryComponent->CraftingRecipeTable || RecipeId.IsNone())
    {
        return false;
    }

    const FCraftingRecipeRow* RecipeRow = InventoryComponent->CraftingRecipeTable->FindRow<FCraftingRecipeRow>(RecipeId, TEXT("TryCraftByRecipeId"));
    if (!RecipeRow || !RecipeRow->ResultItemId.IsValid() || RecipeRow->ResultQuantity <= 0)
    {
        return false;
    }

    for (const FCraftingIngredientRow& Ingredient : RecipeRow->Ingredients)
    {
        if (!Ingredient.ItemId.IsValid() || Ingredient.Quantity <= 0)
        {
            return false;
        }

        if (CountItemInInventory(InventoryComponent, Ingredient.ItemId) < Ingredient.Quantity)
        {
            return false;
        }
    }

    if (!CanStoreResultItem(InventoryComponent, RecipeRow->ResultItemId))
    {
        return false;
    }

    for (const FCraftingIngredientRow& Ingredient : RecipeRow->Ingredients)
    {
        int32 Remaining = Ingredient.Quantity;
        for (FInventorySlotData& Slot : InventoryComponent->InventorySlots)
        {
            if (Remaining <= 0)
            {
                break;
            }

            if (!Slot.IsEmpty() && Slot.Item.ItemId == Ingredient.ItemId)
            {
                const int32 ConsumeCount = FMath::Min(Remaining, Slot.Item.Quantity);
                Slot.Item.Quantity -= ConsumeCount;
                Remaining -= ConsumeCount;

                if (Slot.Item.Quantity <= 0)
                {
                    Slot.Item = FInventoryItemHandle();
                }
            }
        }
    }

    for (FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (!Slot.IsEmpty() && Slot.Item.ItemId == RecipeRow->ResultItemId)
        {
            Slot.Item.Quantity += RecipeRow->ResultQuantity;
            return true;
        }
    }

    for (FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Slot.IsEmpty())
        {
            Slot.Item.ItemId = RecipeRow->ResultItemId;
            Slot.Item.Quantity = RecipeRow->ResultQuantity;
            return true;
        }
    }

    return false;
}
