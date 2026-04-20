#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventoryTypes.generated.h"

class AActor;
class UTexture2D;

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

UENUM(BlueprintType)
enum class EEDInventorySplitMode : uint8
{
    ByCount,
    ByType
};

UENUM(BlueprintType)
enum class EEDInventoryRaritySecondarySplitMode : uint8
{
    NoRaritySorting,
    RaritySorting
};

UENUM(BlueprintType)
enum class EEDInventoryStackPolicy : uint8
{
    Randomize,
    SplitIfPossible,
    UnionAsPossible
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

UENUM(BlueprintType)
enum class EEDSkillSlotType : uint8
{
    FirstSkill,
    SecondSkill
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDSkillSlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EEDSkillSlotType SlotType = EEDSkillSlotType::FirstSkill;

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

UENUM(BlueprintType)
enum class EEDItemDictionarySortOption : uint8
{
    ByRarity,
    ByDisplayName,
    ByItemId,
    ByPrice
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDItemDictionaryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    EEDInventoryItemType ItemType = EEDInventoryItemType::Ingredient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    EEDEquippableType EquippableType = EEDEquippableType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    EEDItemRarity Rarity = EEDItemRarity::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    int32 MaxStack = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    int32 Price = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    FGameplayTagContainer ItemTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Dictionary")
    TObjectPtr<UTexture2D> IconTexture = nullptr;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftTreeFlatNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    int32 NodeId = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    int32 ParentNodeId = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    int32 Depth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FName RowId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FName RecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    bool bIsCraftable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    bool bCyclePruned = false;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftTreePathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    int32 Depth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FName RowId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FName RecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FPrimaryAssetId ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    bool bCyclePruned = false;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftTreePath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FName RootRowId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    FName RootRecipeId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|CraftTree")
    TArray<FEDCraftTreePathNode> Nodes;
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
