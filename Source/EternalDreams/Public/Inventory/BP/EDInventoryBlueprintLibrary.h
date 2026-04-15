#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EDInventoryBlueprintLibrary.generated.h"

class AActor;
class UEDInventoryComponent;

UCLASS()
class ETERNALDREAMS_API UEDInventoryBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    static UEDInventoryComponent* GetInventoryComponentFromActor(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "Inventory")
    static bool IsValidInventorySlotIndex(const UEDInventoryComponent* InventoryComponent, int32 SlotIndex);

    UFUNCTION(BlueprintPure, Category = "Inventory|Failure")
    static bool IsInventoryActionSuccess(EEDInventoryActionFailure Failure);

    UFUNCTION(BlueprintPure, Category = "Inventory|Failure")
    static FText GetInventoryActionFailureText(EEDInventoryActionFailure Failure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Dictionary")
    static void GetItemDictionaryByTags(const FGameplayTagContainer& FilterTags, TArray<FEDItemDictionaryEntry>& OutItems, EEDItemDictionarySortOption SortOption = EEDItemDictionarySortOption::ByRarity, bool bDescending = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft|Viewer")
    static void GetCraftRecipeOptionsForItem(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, TArray<FEDCraftableRecipeEntry>& OutRecipes);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft|Viewer")
    static void BuildCraftTreeFlat(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, TArray<FEDCraftTreeFlatNode>& OutNodes, bool& bOutCyclePruned, int32 MaxDepth = 8);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft|Viewer")
    static void BuildCraftTreePaths(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, TArray<FEDCraftTreePath>& OutPaths, bool& bOutCyclePruned, int32 MaxDepth = 8);
};
