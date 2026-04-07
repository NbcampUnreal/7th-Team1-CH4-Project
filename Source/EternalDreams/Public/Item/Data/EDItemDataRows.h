#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Item/Core/EDItemTypes.h"
#include "EDItemDataRows.generated.h"

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftingIngredientRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowedTypes = "InventoryItem"))
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftingRecipeRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEDCraftingIngredientRow> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowedTypes = "InventoryItem"))
    FPrimaryAssetId ResultItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 ResultQuantity = 1;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDItemSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowedTypes = "InventoryItem"))
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDItemRarityVisualRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEDItemRarity Rarity = EEDItemRarity::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DisplayColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SortOrder = 0;
};
