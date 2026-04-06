#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"
#include "ItemTypes.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
    Equippable,
    Consumable,
    Ingredient
};

UENUM(BlueprintType)
enum class EEquippableType : uint8
{
    None,
    Weapon,
    TopArmor,
    BottomArmor
};

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Normal,
    Rare,
    Epic,
    Legendary,
    Unique
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FInventoryItemHandle
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
    int32 Quantity = 0;

    bool IsValid() const
    {
        return ItemId.IsValid() && Quantity > 0;
    }
};
