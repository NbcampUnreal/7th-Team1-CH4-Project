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

    // 제작 UI에서 바로 사용할 수 있는 레시피 목록을 반환
    // 각 레시피에는 결과 아이템 정보와 재료별 보유 상태가 함께 들어 있음
    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft|Viewer")
    static void GetCraftRecipeViewDataList(UEDInventoryComponent* InventoryComponent, TArray<FEDCraftRecipeViewData>& OutRecipes, bool bOnlyCraftable = false, EEDCraftableRecipeSortOption SortOption = EEDCraftableRecipeSortOption::ByRowId, bool bDescending = false);

    // 특정 RowId의 레시피 하나를 제작 UI용 데이터 형태로 반환
    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft|Viewer")
    static bool GetCraftRecipeViewDataByRowId(UEDInventoryComponent* InventoryComponent, FName RecipeRowId, FEDCraftRecipeViewData& OutRecipe);
};
