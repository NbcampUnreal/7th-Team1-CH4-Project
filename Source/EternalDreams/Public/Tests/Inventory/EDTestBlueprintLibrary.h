#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EDTestBlueprintLibrary.generated.h"

class AActor;
class UObject;
class UEDInventoryComponent;
class UTexture2D;
class UDataTable;

UCLASS()
class ETERNALDREAMS_API UEDTestBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "ED|Test|Actor")
    static AActor* GetClosestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Actor")
    static AActor* GetFarthestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory", meta = (WorldContext = "WorldContextObject"))
    static FString BuildAllInventoryDebugText(const UObject* WorldContextObject, bool bOnlyAuthority = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory")
    static FString BuildInventoryDebugTextFromArray(const TArray<UEDInventoryComponent*>& InventoryComponents, bool bOnlyAuthority = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory", meta = (WorldContext = "WorldContextObject"))
    static void GetAllWorldInventoryComponents(const UObject* WorldContextObject, TArray<UEDInventoryComponent*>& OutInventoryComponents, bool bOnlyAuthority = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Inventory", meta = (WorldContext = "WorldContextObject"))
    static void PrintAllInventoryDebugText(const UObject* WorldContextObject, float Duration = 0.0f, bool bOnlyAuthority = false);

    UFUNCTION(BlueprintPure, Category = "ED|Test|Item")
    static UTexture2D* GetItemIconByAssetId(FPrimaryAssetId ItemId);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Item", meta = (AutoCreateRefTerm = "FilterTags"))
    static FString BuildItemDictionaryDebugStringByTags(const FGameplayTagContainer& FilterTags, EEDItemDictionarySortOption SortOption = EEDItemDictionarySortOption::ByRarity, bool bDescending = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Item", meta = (AutoCreateRefTerm = "FilterTags"))
    static FText BuildItemDictionaryDebugTextByTags(const FGameplayTagContainer& FilterTags, EEDItemDictionarySortOption SortOption = EEDItemDictionarySortOption::ByRarity, bool bDescending = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Craft")
    static FString BuildCraftRecipeDebugString(UEDInventoryComponent* InventoryComponent, bool bOnlyCraftable = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Craft")
    static FText BuildCraftRecipeDebugText(UEDInventoryComponent* InventoryComponent, bool bOnlyCraftable = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Craft")
    static void PrintCraftRecipeDebugText(UEDInventoryComponent* InventoryComponent, float Duration = 8.0f, bool bOnlyCraftable = false);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Craft")
    static FString BuildCraftTreeDebugStringByItemId(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, int32 MaxDepth = 8);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Craft")
    static FText BuildCraftTreeDebugTextByItemId(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, int32 MaxDepth = 8);

    UFUNCTION(BlueprintCallable, Category = "ED|Test|Craft")
    static void PrintCraftTreeDebugTextByItemId(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, float Duration = 8.0f, int32 MaxDepth = 8);
};
