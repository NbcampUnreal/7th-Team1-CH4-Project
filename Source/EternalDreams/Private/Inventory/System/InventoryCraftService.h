#pragma once

#include "CoreMinimal.h"

class UInventoryComponent;

class FInventoryCraftService
{
public:
    static bool TryCraftByRecipeId(UInventoryComponent* InventoryComponent, FName RecipeId);
};

