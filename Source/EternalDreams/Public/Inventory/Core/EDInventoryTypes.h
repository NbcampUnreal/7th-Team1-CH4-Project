#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventoryTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EEDInventoryDropReason : uint8
{
    UserRequested,
    UnequipNoSpace,
    CraftSwap,
    System
};

UENUM(BlueprintType)
enum class EEDInventoryActionFailure : uint8
{
    None,
    InvalidInventory,
    InvalidSlot,
    EmptySlot,
    InvalidQuantity,
    SlotConflict,
    NoSpace,
    StackLimit,
    MissingData,
    InvalidRecipe,
    MissingIngredient,
    NotConsumable,
    HealthAlreadyFull,
    EffectApplyFailed
};

UENUM(BlueprintType)
enum class EEDInventoryLootSpawnMode : uint8
{
    Static,
    QuantityMax,
    RollCountMax
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDInventorySlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FEDInventoryItemHandle Item;

    bool IsEmpty() const
    {
        return !Item.IsValid();
    }
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDEquipmentSlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EEDEquippableType SlotType = EEDEquippableType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FEDInventoryItemHandle EquippedItem;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDInventoryDropRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FEDInventoryItemHandle Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TObjectPtr<AActor> SourceOwner = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EEDInventoryDropReason Reason = EEDInventoryDropReason::UserRequested;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FGameplayTagContainer ContextTags;
};

UENUM(BlueprintType)
enum class EEDCraftableRecipeSortOption : uint8
{
    ByRowId,
    ByRarity,
    ByResultItemId,
    ByResultItemName
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftableRecipeEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft")
    FName RowId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft")
    FName RecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft")
    FPrimaryAssetId ResultItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft")
    FText ResultItemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft")
    EEDItemRarity ResultRarity = EEDItemRarity::Normal;
};
