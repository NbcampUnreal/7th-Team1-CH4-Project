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
    SplitByCount,
    SplitByType,
    SplitByRarity
};

UENUM(BlueprintType)
enum class EEDInventoryRaritySecondarySplitMode : uint8
{
    ByCount,
    ByType
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

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftIngredientViewData
{
    GENERATED_BODY()

    // 재료 아이템 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    FPrimaryAssetId ItemId;

    // UI에 표시할 재료 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    FText DisplayName;

    // 재료 희귀도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    EEDItemRarity Rarity = EEDItemRarity::Normal;

    // 재료 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    TObjectPtr<UTexture2D> IconTexture = nullptr;

    // 제작에 필요한 재료 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    int32 RequiredQuantity = 1;

    // 현재 플레이어가 보유한 재료 수량
    // 인벤토리와 장비 슬롯에 있는 수량을 합산한 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    int32 OwnedQuantity = 0;

    // 필요 수량을 충족했는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    bool bSatisfied = false;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftRecipeViewData
{
    GENERATED_BODY()

    // 데이터 테이블의 실제 Row 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    FName RowId = NAME_None;

    // 레시피 식별용 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    FName RecipeId = NAME_None;

    // 결과 아이템 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    FPrimaryAssetId ResultItemId;

    // UI에 표시할 결과 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    FText ResultItemName;

    // 결과 아이템 희귀도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    EEDItemRarity ResultRarity = EEDItemRarity::Normal;

    // 결과 아이템 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    TObjectPtr<UTexture2D> ResultIconTexture = nullptr;

    // 한 번 제작했을 때 생성되는 결과 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    int32 ResultQuantity = 1;

    // 현재 인벤토리 상태 기준으로 이 레시피를 바로 제작할 수 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    bool bCanCraft = false;

    // 이 레시피를 구성하는 재료 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|View")
    TArray<FEDCraftIngredientViewData> Ingredients;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDCraftTreeNodeViewData
{
    GENERATED_BODY()

    // 트리 내부에서 노드를 구분하기 위한 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    int32 NodeId = -1;

    // 부모 노드 ID. 루트 노드는 -1을 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    int32 ParentNodeId = -1;

    // 트리에서 몇 단계 아래에 있는지 나타내는 깊이 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    int32 Depth = 0;

    // 이 노드가 가리키는 아이템 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    FPrimaryAssetId ItemId;

    // UI에 표시할 아이템 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    FText DisplayName;

    // UI에 표시할 아이템 희귀도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    EEDItemRarity Rarity = EEDItemRarity::Normal;

    // UI에 표시할 아이콘 텍스처
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    TObjectPtr<UTexture2D> IconTexture = nullptr;

    // 이 노드가 요구하는 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    int32 RequiredQuantity = 1;

    // 현재 플레이어가 보유한 수량. 인벤토리와 장비 슬롯을 합산한 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    int32 OwnedQuantity = 0;

    // 요구 수량을 충족했는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    bool bSatisfied = false;

    // 이 노드 아이템 자체가 하위 재료로 다시 제작 가능한지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Craft|TreeView")
    bool bIsCraftable = false;
};
