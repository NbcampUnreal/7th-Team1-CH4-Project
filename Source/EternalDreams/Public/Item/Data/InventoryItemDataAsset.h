#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Item/Core/ItemTypes.h"
#include "InventoryItemDataAsset.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType)
class ETERNALDREAMS_API UInventoryItemDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FPrimaryAssetId ItemId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EInventoryItemType ItemType = EInventoryItemType::Ingredient;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EEquippableType EquippableType = EEquippableType::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EItemRarity Rarity = EItemRarity::Normal;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
    int32 MaxStack = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
    int32 Price = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTagContainer ItemTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable")
    TSubclassOf<UGameplayEffect> ConsumableEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Consumable", meta = (ClampMin = "0.0"))
    float ConsumableDuration = 0.0f;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return ItemId;
    }
};
