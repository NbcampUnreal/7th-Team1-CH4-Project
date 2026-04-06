#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Item/Core/ItemTypes.h"
#include "ItemDataRows.generated.h"

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FCraftingIngredientRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FCraftingRecipeRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCraftingIngredientRow> Ingredients;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPrimaryAssetId ResultItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 ResultQuantity = 1;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FItemSpawnRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
    int32 MaxCount = 1;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FItemRarityVisualRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemRarity Rarity = EItemRarity::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DisplayColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SortOrder = 0;
};

