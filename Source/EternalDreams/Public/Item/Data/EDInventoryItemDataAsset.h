#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventoryItemDataAsset.generated.h"

class UGameplayEffect;

UCLASS(BlueprintType)
class ETERNALDREAMS_API UEDInventoryItemDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EEDInventoryItemType ItemType = EEDInventoryItemType::Ingredient;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EEDEquippableType EquippableType = EEDEquippableType::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    EEDItemRarity Rarity = EEDItemRarity::Normal;

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Equip")
    TSubclassOf<UGameplayEffect> EquipEffectClass;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        static const FPrimaryAssetType ItemAssetType(TEXT("InventoryItem"));
        return FPrimaryAssetId(ItemAssetType, GetFName());
    }
};
