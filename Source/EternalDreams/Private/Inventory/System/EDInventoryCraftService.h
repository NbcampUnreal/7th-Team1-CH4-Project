#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"

struct FEDCraftingRecipeRow;
class UEDInventoryComponent;

class FEDInventoryCraftService
{
public:
    static bool TryCraftByRecipeId(UEDInventoryComponent* InventoryComponent, FName RecipeId, EEDInventoryActionFailure* OutFailure = nullptr);
    static bool CanCraftRecipe(const UEDInventoryComponent* InventoryComponent, const FEDCraftingRecipeRow& RecipeRow, EEDInventoryActionFailure* OutFailure = nullptr);
    static void GetCraftableRecipes(const UEDInventoryComponent* InventoryComponent, TArray<FEDCraftableRecipeEntry>& OutRecipes, EEDCraftableRecipeSortOption SortOption, bool bDescending = false);
};
