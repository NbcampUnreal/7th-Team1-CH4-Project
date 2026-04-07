#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"

class UEDInventoryComponent;

class FEDInventoryCraftService
{
public:
    static bool TryCraftByRecipeId(UEDInventoryComponent* InventoryComponent, FName RecipeId, EEDInventoryActionFailure* OutFailure = nullptr);
};
