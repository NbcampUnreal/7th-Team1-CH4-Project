#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/InventoryTypes.h"

class UInventoryComponent;

class FInventoryCraftService
{
public:
    static bool TryCraftByRecipeId(UInventoryComponent* InventoryComponent, FName RecipeId, EInventoryActionFailure* OutFailure = nullptr);
};
