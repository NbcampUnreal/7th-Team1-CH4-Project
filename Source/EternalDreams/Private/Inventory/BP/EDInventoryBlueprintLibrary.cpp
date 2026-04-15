#include "Inventory/BP/EDInventoryBlueprintLibrary.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Internationalization/Text.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/Interface/EDInventoryProviderInterface.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Item/Data/EDItemDataRows.h"

namespace
{
struct FCraftRecipeViewRow
{
    FName RowId = NAME_None;
    FName RecipeId = NAME_None;
    FPrimaryAssetId ResultItemId;
    int32 ResultQuantity = 1;
    TArray<FEDCraftingIngredientRow> Ingredients;
};

const UEDInventoryItemDataAsset* ResolveItemData_BP(const FPrimaryAssetId& ItemId)
{
    if (!ItemId.IsValid())
    {
        return nullptr;
    }

    UObject* ItemObject = UAssetManager::Get().GetPrimaryAssetObject(ItemId);
    if (!ItemObject)
    {
        const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
        if (AssetPath.IsValid())
        {
            ItemObject = AssetPath.TryLoad();
        }
    }

    return Cast<UEDInventoryItemDataAsset>(ItemObject);
}

FText ResolveDisplayName_BP(const FPrimaryAssetId& ItemId)
{
    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_BP(ItemId);
    if (ItemData && !ItemData->DisplayName.IsEmpty())
    {
        return ItemData->DisplayName;
    }

    return FText::FromName(ItemId.PrimaryAssetName);
}

bool MatchesAnyParentTag(const FGameplayTagContainer& ItemTags, const FGameplayTagContainer& FilterTags)
{
    if (FilterTags.Num() <= 0)
    {
        return true;
    }

    for (const FGameplayTag& FilterTag : FilterTags)
    {
        if (!FilterTag.IsValid())
        {
            continue;
        }

        for (const FGameplayTag& ItemTag : ItemTags)
        {
            if (ItemTag.IsValid() && ItemTag.MatchesTag(FilterTag))
            {
                return true;
            }
        }
    }

    return false;
}

int32 CompareItemDictionaryEntry(const FEDItemDictionaryEntry& A, const FEDItemDictionaryEntry& B, EEDItemDictionarySortOption SortOption)
{
    auto CompareText = [](const FText& Left, const FText& Right) -> int32
    {
        return FCString::Stricmp(*Left.ToString(), *Right.ToString());
    };

    switch (SortOption)
    {
    case EEDItemDictionarySortOption::ByDisplayName:
        {
            const int32 NameCompare = CompareText(A.DisplayName, B.DisplayName);
            if (NameCompare != 0)
            {
                return NameCompare;
            }
        }
        break;

    case EEDItemDictionarySortOption::ByItemId:
        {
            const int32 IdCompare = FCString::Stricmp(*A.ItemId.ToString(), *B.ItemId.ToString());
            if (IdCompare != 0)
            {
                return IdCompare;
            }
        }
        break;

    case EEDItemDictionarySortOption::ByPrice:
        if (A.Price != B.Price)
        {
            return A.Price - B.Price;
        }
        break;

    case EEDItemDictionarySortOption::ByRarity:
    default:
        if (A.Rarity != B.Rarity)
        {
            return static_cast<int32>(A.Rarity) - static_cast<int32>(B.Rarity);
        }
        break;
    }

    const int32 NameCompare = CompareText(A.DisplayName, B.DisplayName);
    if (NameCompare != 0)
    {
        return NameCompare;
    }

    return FCString::Stricmp(*A.ItemId.ToString(), *B.ItemId.ToString());
}

int32 CountItemInInventory_BP(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    // 일반 인벤토리 슬롯 안에 있는 같은 아이템 수량을 모두 합산
    if (!InventoryComponent || !ItemId.IsValid())
    {
        return 0;
    }

    int32 Count = 0;
    for (const FEDInventorySlotData& SlotData : InventoryComponent->InventorySlots)
    {
        if (!SlotData.IsEmpty() && SlotData.Item.ItemId == ItemId)
        {
            Count += SlotData.Item.Quantity;
        }
    }

    return Count;
}

int32 CountItemInEquipment_BP(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    // 무기 / 상의 / 하의 장비 슬롯에 장착된 같은 아이템 수량을 합산
    if (!InventoryComponent || !ItemId.IsValid())
    {
        return 0;
    }

    int32 Count = 0;
    const TArray<const FEDEquipmentSlotData*> EquipmentSlots =
    {
        &InventoryComponent->WeaponSlot,
        &InventoryComponent->TopArmorSlot,
        &InventoryComponent->BottomArmorSlot
    };

    for (const FEDEquipmentSlotData* EquipmentSlot : EquipmentSlots)
    {
        if (EquipmentSlot && EquipmentSlot->EquippedItem.IsValid() && EquipmentSlot->EquippedItem.ItemId == ItemId)
        {
            Count += EquipmentSlot->EquippedItem.Quantity;
        }
    }

    return Count;
}

int32 CountItemTotal_BP(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    // 제작은 장착 중인 장비도 재료로 소비할 수 있으므로,
    // 인벤토리 수량과 장비 슬롯 수량을 합친 총량을 사용
    return CountItemInInventory_BP(InventoryComponent, ItemId) + CountItemInEquipment_BP(InventoryComponent, ItemId);
}

int32 CompareCraftRecipeViewData(const FEDCraftRecipeViewData& A, const FEDCraftRecipeViewData& B, EEDCraftableRecipeSortOption SortOption)
{
    // 제작 UI 리스트 정렬에 사용할 비교 함수
    auto CompareText = [](const FText& Left, const FText& Right) -> int32
    {
        return FCString::Stricmp(*Left.ToString(), *Right.ToString());
    };

    switch (SortOption)
    {
    case EEDCraftableRecipeSortOption::ByRarity:
        if (A.ResultRarity != B.ResultRarity)
        {
            return static_cast<int32>(A.ResultRarity) - static_cast<int32>(B.ResultRarity);
        }
        break;

    case EEDCraftableRecipeSortOption::ByResultItemId:
        {
            const int32 IdCompare = FCString::Stricmp(*A.ResultItemId.ToString(), *B.ResultItemId.ToString());
            if (IdCompare != 0)
            {
                return IdCompare;
            }
        }
        break;

    case EEDCraftableRecipeSortOption::ByResultItemName:
        {
            const int32 NameCompare = CompareText(A.ResultItemName, B.ResultItemName);
            if (NameCompare != 0)
            {
                return NameCompare;
            }
        }
        break;

    case EEDCraftableRecipeSortOption::ByRowId:
    default:
        break;
    }

    const int32 RowCompare = FCString::Stricmp(*A.RowId.ToString(), *B.RowId.ToString());
    if (RowCompare != 0)
    {
        return RowCompare;
    }

    return FCString::Stricmp(*A.RecipeId.ToString(), *B.RecipeId.ToString());
}

bool BuildCraftRecipeViewData(
    UEDInventoryComponent* InventoryComponent,
    const FCraftRecipeViewRow& Recipe,
    FEDCraftRecipeViewData& OutRecipe)
{
    // 데이터 테이블의 레시피 1개를
    // UI에서 바로 그릴 수 있는 형태로 변환
    if (!InventoryComponent || !Recipe.ResultItemId.IsValid())
    {
        return false;
    }

    OutRecipe = FEDCraftRecipeViewData();
    OutRecipe.RowId = Recipe.RowId;
    OutRecipe.RecipeId = Recipe.RecipeId;
    OutRecipe.ResultItemId = Recipe.ResultItemId;
    OutRecipe.ResultItemName = ResolveDisplayName_BP(Recipe.ResultItemId);
    OutRecipe.ResultQuantity = Recipe.ResultQuantity;
    OutRecipe.bCanCraft = InventoryComponent->CanCraftRecipeByRowId(Recipe.RowId);

    const UEDInventoryItemDataAsset* ResultData = ResolveItemData_BP(Recipe.ResultItemId);
    if (ResultData)
    {
        OutRecipe.ResultRarity = ResultData->Rarity;
        OutRecipe.ResultIconTexture = ResultData->IconTexture;
    }

    for (const FEDCraftingIngredientRow& Ingredient : Recipe.Ingredients)
    {
        if (!Ingredient.ItemId.IsValid() || Ingredient.Quantity <= 0)
        {
            continue;
        }

        FEDCraftIngredientViewData IngredientView;
        IngredientView.ItemId = Ingredient.ItemId;
        IngredientView.DisplayName = ResolveDisplayName_BP(Ingredient.ItemId);
        IngredientView.RequiredQuantity = Ingredient.Quantity;

        // 현재 플레이어가 실제로 가진 수량을 계산
        // 장비 슬롯에 있는 아이템도 포함
        IngredientView.OwnedQuantity = CountItemTotal_BP(InventoryComponent, Ingredient.ItemId);

        // 필요 수량 이상 보유했는지 여부를 미리 계산
        // UI에서는 이 값을 이용해 강조 색상 등을 바로 적용
        IngredientView.bSatisfied = IngredientView.OwnedQuantity >= IngredientView.RequiredQuantity;

        const UEDInventoryItemDataAsset* IngredientData = ResolveItemData_BP(Ingredient.ItemId);
        if (IngredientData)
        {
            IngredientView.Rarity = IngredientData->Rarity;
            IngredientView.IconTexture = IngredientData->IconTexture;
        }

        OutRecipe.Ingredients.Add(MoveTemp(IngredientView));
    }

    return true;
}

void GatherCraftRecipes(UEDInventoryComponent* InventoryComponent, TArray<FCraftRecipeViewRow>& OutRecipes)
{
    OutRecipes.Reset();

    if (!InventoryComponent)
    {
        return;
    }

    TArray<UDataTable*> RecipeTables;
    InventoryComponent->GetAllCraftingRecipeTables(RecipeTables);

    for (UDataTable* RecipeTable : RecipeTables)
    {
        if (!RecipeTable)
        {
            continue;
        }

        const TArray<FName> RowNames = RecipeTable->GetRowNames();
        for (const FName RowName : RowNames)
        {
            const FEDCraftingRecipeRow* Row = RecipeTable->FindRow<FEDCraftingRecipeRow>(RowName, TEXT("GatherCraftRecipes"));
            if (!Row || !Row->ResultItemId.IsValid() || Row->ResultQuantity <= 0)
            {
                continue;
            }

            FCraftRecipeViewRow ViewRow;
            ViewRow.RowId = RowName;
            ViewRow.RecipeId = Row->RecipeId;
            ViewRow.ResultItemId = Row->ResultItemId;
            ViewRow.ResultQuantity = Row->ResultQuantity;
            ViewRow.Ingredients = Row->Ingredients;
            OutRecipes.Add(MoveTemp(ViewRow));
        }
    }
}

void ExpandCraftFlatRecursive(
    const FPrimaryAssetId& ItemId,
    int32 Quantity,
    int32 ParentNodeId,
    int32 Depth,
    FName ViaRowId,
    FName ViaRecipeId,
    const TArray<FCraftRecipeViewRow>& Recipes,
    TSet<FPrimaryAssetId>& ActivePath,
    int32 MaxDepth,
    int32& InOutNodeId,
    TArray<FEDCraftTreeFlatNode>& OutNodes,
    bool& bOutCyclePruned)
{
    FEDCraftTreeFlatNode Node;
    Node.NodeId = InOutNodeId++;
    Node.ParentNodeId = ParentNodeId;
    Node.Depth = Depth;
    Node.RowId = ViaRowId;
    Node.RecipeId = ViaRecipeId;
    Node.ItemId = ItemId;
    Node.Quantity = FMath::Max(1, Quantity);

    TArray<const FCraftRecipeViewRow*> MatchingRecipes;
    if (Depth < MaxDepth)
    {
        for (const FCraftRecipeViewRow& Recipe : Recipes)
        {
            if (Recipe.ResultItemId == ItemId)
            {
                MatchingRecipes.Add(&Recipe);
            }
        }
    }

    Node.bIsCraftable = MatchingRecipes.Num() > 0;
    const int32 CurrentNodeIndex = OutNodes.Add(Node);

    if (MatchingRecipes.Num() <= 0)
    {
        return;
    }

    for (const FCraftRecipeViewRow* Recipe : MatchingRecipes)
    {
        if (!Recipe)
        {
            continue;
        }

        for (const FEDCraftingIngredientRow& Ingredient : Recipe->Ingredients)
        {
            if (!Ingredient.ItemId.IsValid() || Ingredient.Quantity <= 0)
            {
                continue;
            }

            if (ActivePath.Contains(Ingredient.ItemId))
            {
                bOutCyclePruned = true;

                FEDCraftTreeFlatNode PrunedNode;
                PrunedNode.NodeId = InOutNodeId++;
                PrunedNode.ParentNodeId = OutNodes[CurrentNodeIndex].NodeId;
                PrunedNode.Depth = Depth + 1;
                PrunedNode.RowId = Recipe->RowId;
                PrunedNode.RecipeId = Recipe->RecipeId;
                PrunedNode.ItemId = Ingredient.ItemId;
                PrunedNode.Quantity = Ingredient.Quantity;
                PrunedNode.bIsCraftable = true;
                PrunedNode.bCyclePruned = true;
                OutNodes.Add(MoveTemp(PrunedNode));
                continue;
            }

            ActivePath.Add(Ingredient.ItemId);
            ExpandCraftFlatRecursive(
                Ingredient.ItemId,
                Ingredient.Quantity,
                OutNodes[CurrentNodeIndex].NodeId,
                Depth + 1,
                Recipe->RowId,
                Recipe->RecipeId,
                Recipes,
                ActivePath,
                MaxDepth,
                InOutNodeId,
                OutNodes,
                bOutCyclePruned);
            ActivePath.Remove(Ingredient.ItemId);
        }
    }
}

void BuildPathRecursive(
    const FPrimaryAssetId& ItemId,
    int32 Quantity,
    int32 Depth,
    FName ViaRowId,
    FName ViaRecipeId,
    FName RootRowId,
    FName RootRecipeId,
    const TArray<FCraftRecipeViewRow>& Recipes,
    TSet<FPrimaryAssetId>& ActivePath,
    int32 MaxDepth,
    TArray<FEDCraftTreePathNode>& CurrentPath,
    TArray<FEDCraftTreePath>& OutPaths,
    bool& bOutCyclePruned)
{
    FEDCraftTreePathNode PathNode;
    PathNode.Depth = Depth;
    PathNode.RowId = ViaRowId;
    PathNode.RecipeId = ViaRecipeId;
    PathNode.ItemId = ItemId;
    PathNode.Quantity = FMath::Max(1, Quantity);
    CurrentPath.Add(PathNode);

    TArray<const FCraftRecipeViewRow*> MatchingRecipes;
    if (Depth < MaxDepth)
    {
        for (const FCraftRecipeViewRow& Recipe : Recipes)
        {
            if (Recipe.ResultItemId == ItemId)
            {
                MatchingRecipes.Add(&Recipe);
            }
        }
    }

    if (MatchingRecipes.Num() <= 0)
    {
        FEDCraftTreePath NewPath;
        NewPath.RootRowId = RootRowId;
        NewPath.RootRecipeId = RootRecipeId;
        NewPath.Nodes = CurrentPath;
        OutPaths.Add(MoveTemp(NewPath));
        CurrentPath.Pop();
        return;
    }

    for (const FCraftRecipeViewRow* Recipe : MatchingRecipes)
    {
        if (!Recipe)
        {
            continue;
        }

        bool bAddedBranch = false;
        for (const FEDCraftingIngredientRow& Ingredient : Recipe->Ingredients)
        {
            if (!Ingredient.ItemId.IsValid() || Ingredient.Quantity <= 0)
            {
                continue;
            }

            bAddedBranch = true;

            if (ActivePath.Contains(Ingredient.ItemId))
            {
                bOutCyclePruned = true;

                FEDCraftTreePathNode PrunedNode;
                PrunedNode.Depth = Depth + 1;
                PrunedNode.RowId = Recipe->RowId;
                PrunedNode.RecipeId = Recipe->RecipeId;
                PrunedNode.ItemId = Ingredient.ItemId;
                PrunedNode.Quantity = Ingredient.Quantity;
                PrunedNode.bCyclePruned = true;

                CurrentPath.Add(PrunedNode);
                FEDCraftTreePath NewPath;
                NewPath.RootRowId = RootRowId;
                NewPath.RootRecipeId = RootRecipeId;
                NewPath.Nodes = CurrentPath;
                OutPaths.Add(MoveTemp(NewPath));
                CurrentPath.Pop();
                continue;
            }

            ActivePath.Add(Ingredient.ItemId);
            BuildPathRecursive(
                Ingredient.ItemId,
                Ingredient.Quantity,
                Depth + 1,
                Recipe->RowId,
                Recipe->RecipeId,
                RootRowId,
                RootRecipeId,
                Recipes,
                ActivePath,
                MaxDepth,
                CurrentPath,
                OutPaths,
                bOutCyclePruned);
            ActivePath.Remove(Ingredient.ItemId);
        }

        if (!bAddedBranch)
        {
            FEDCraftTreePath NewPath;
            NewPath.RootRowId = RootRowId;
            NewPath.RootRecipeId = RootRecipeId;
            NewPath.Nodes = CurrentPath;
            OutPaths.Add(MoveTemp(NewPath));
        }
    }

    CurrentPath.Pop();
}
}

UEDInventoryComponent* UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }

    if (Actor->GetClass()->ImplementsInterface(UEDInventoryProviderInterface::StaticClass()))
    {
        return IEDInventoryProviderInterface::Execute_GetInventoryComponent(Actor);
    }

    return Actor->FindComponentByClass<UEDInventoryComponent>();
}

bool UEDInventoryBlueprintLibrary::IsValidInventorySlotIndex(const UEDInventoryComponent* InventoryComponent, int32 SlotIndex)
{
    return InventoryComponent && InventoryComponent->InventorySlots.IsValidIndex(SlotIndex);
}

bool UEDInventoryBlueprintLibrary::IsInventoryActionSuccess(EEDInventoryActionFailure Failure)
{
    return Failure == EEDInventoryActionFailure::None;
}

FText UEDInventoryBlueprintLibrary::GetInventoryActionFailureText(EEDInventoryActionFailure Failure)
{
    switch (Failure)
    {
    case EEDInventoryActionFailure::None:
        return NSLOCTEXT("Inventory", "FailureNone", "No error");
    case EEDInventoryActionFailure::InvalidInventory:
        return NSLOCTEXT("Inventory", "FailureInvalidInventory", "Invalid inventory");
    case EEDInventoryActionFailure::InvalidSlot:
        return NSLOCTEXT("Inventory", "FailureInvalidSlot", "Invalid slot");
    case EEDInventoryActionFailure::EmptySlot:
        return NSLOCTEXT("Inventory", "FailureEmptySlot", "Selected slot is empty");
    case EEDInventoryActionFailure::InvalidQuantity:
        return NSLOCTEXT("Inventory", "FailureInvalidQuantity", "Invalid quantity");
    case EEDInventoryActionFailure::SlotConflict:
        return NSLOCTEXT("Inventory", "FailureSlotConflict", "Slot conflict");
    case EEDInventoryActionFailure::NoSpace:
        return NSLOCTEXT("Inventory", "FailureNoSpace", "No space available");
    case EEDInventoryActionFailure::StackLimit:
        return NSLOCTEXT("Inventory", "FailureStackLimit", "Stack limit reached");
    case EEDInventoryActionFailure::MissingData:
        return NSLOCTEXT("Inventory", "FailureMissingData", "Missing item data");
    case EEDInventoryActionFailure::InvalidRecipe:
        return NSLOCTEXT("Inventory", "FailureInvalidRecipe", "Invalid recipe");
    case EEDInventoryActionFailure::MissingIngredient:
        return NSLOCTEXT("Inventory", "FailureMissingIngredient", "Missing ingredient");
    case EEDInventoryActionFailure::NotConsumable:
        return NSLOCTEXT("Inventory", "FailureNotConsumable", "Item is not consumable");
    case EEDInventoryActionFailure::HealthAlreadyFull:
        return NSLOCTEXT("Inventory", "FailureHealthAlreadyFull", "Health is already full");
    case EEDInventoryActionFailure::EffectApplyFailed:
        return NSLOCTEXT("Inventory", "FailureEffectApplyFailed", "Failed to apply effect");
    default:
        return NSLOCTEXT("Inventory", "FailureUnknown", "Unknown failure");
    }
}

void UEDInventoryBlueprintLibrary::GetItemDictionaryByTags(const FGameplayTagContainer& FilterTags, TArray<FEDItemDictionaryEntry>& OutItems, EEDItemDictionarySortOption SortOption, bool bDescending)
{
    OutItems.Reset();

    TArray<FPrimaryAssetId> ItemIds;
    UAssetManager::Get().GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("InventoryItem")), ItemIds);

    for (const FPrimaryAssetId& ItemId : ItemIds)
    {
        const UEDInventoryItemDataAsset* ItemData = ResolveItemData_BP(ItemId);
        if (!ItemData)
        {
            continue;
        }

        if (!MatchesAnyParentTag(ItemData->ItemTags, FilterTags))
        {
            continue;
        }

        FEDItemDictionaryEntry Entry;
        Entry.ItemId = ItemId;
        Entry.DisplayName = ItemData->DisplayName.IsEmpty() ? FText::FromName(ItemId.PrimaryAssetName) : ItemData->DisplayName;
        Entry.ItemType = ItemData->ItemType;
        Entry.EquippableType = ItemData->EquippableType;
        Entry.Rarity = ItemData->Rarity;
        Entry.MaxStack = ItemData->MaxStack;
        Entry.Price = ItemData->Price;
        Entry.ItemTags = ItemData->ItemTags;
        Entry.IconTexture = ItemData->IconTexture;

        OutItems.Add(MoveTemp(Entry));
    }

    OutItems.Sort([SortOption, bDescending](const FEDItemDictionaryEntry& A, const FEDItemDictionaryEntry& B)
    {
        const int32 Compare = CompareItemDictionaryEntry(A, B, SortOption);
        return bDescending ? (Compare > 0) : (Compare < 0);
    });
}

void UEDInventoryBlueprintLibrary::GetCraftRecipeOptionsForItem(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, TArray<FEDCraftableRecipeEntry>& OutRecipes)
{
    OutRecipes.Reset();

    if (!InventoryComponent || !ResultItemId.IsValid())
    {
        return;
    }

    TArray<FCraftRecipeViewRow> Recipes;
    GatherCraftRecipes(InventoryComponent, Recipes);

    for (const FCraftRecipeViewRow& Recipe : Recipes)
    {
        if (Recipe.ResultItemId != ResultItemId)
        {
            continue;
        }

        FEDCraftableRecipeEntry Entry;
        Entry.RowId = Recipe.RowId;
        Entry.RecipeId = Recipe.RecipeId;
        Entry.ResultItemId = Recipe.ResultItemId;
        Entry.ResultItemName = ResolveDisplayName_BP(Recipe.ResultItemId);

        const UEDInventoryItemDataAsset* ResultData = ResolveItemData_BP(Recipe.ResultItemId);
        Entry.ResultRarity = ResultData ? ResultData->Rarity : EEDItemRarity::Normal;
        OutRecipes.Add(MoveTemp(Entry));
    }

    OutRecipes.Sort([](const FEDCraftableRecipeEntry& A, const FEDCraftableRecipeEntry& B)
    {
        if (A.RowId != B.RowId)
        {
            return FCString::Stricmp(*A.RowId.ToString(), *B.RowId.ToString()) < 0;
        }

        return FCString::Stricmp(*A.RecipeId.ToString(), *B.RecipeId.ToString()) < 0;
    });
}

void UEDInventoryBlueprintLibrary::BuildCraftTreeFlat(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, TArray<FEDCraftTreeFlatNode>& OutNodes, bool& bOutCyclePruned, int32 MaxDepth)
{
    OutNodes.Reset();
    bOutCyclePruned = false;

    if (!InventoryComponent || !ResultItemId.IsValid())
    {
        return;
    }

    TArray<FCraftRecipeViewRow> Recipes;
    GatherCraftRecipes(InventoryComponent, Recipes);

    const int32 ClampedMaxDepth = FMath::Clamp(MaxDepth, 1, 8);
    int32 NextNodeId = 0;

    TSet<FPrimaryAssetId> ActivePath;
    ActivePath.Add(ResultItemId);

    ExpandCraftFlatRecursive(
        ResultItemId,
        1,
        INDEX_NONE,
        0,
        NAME_None,
        NAME_None,
        Recipes,
        ActivePath,
        ClampedMaxDepth,
        NextNodeId,
        OutNodes,
        bOutCyclePruned);

    OutNodes.Sort([](const FEDCraftTreeFlatNode& A, const FEDCraftTreeFlatNode& B)
    {
        if (A.Depth != B.Depth)
        {
            return A.Depth < B.Depth;
        }

        const int32 RowCompare = FCString::Stricmp(*A.RowId.ToString(), *B.RowId.ToString());
        if (RowCompare != 0)
        {
            return RowCompare < 0;
        }

        return FCString::Stricmp(*A.ItemId.ToString(), *B.ItemId.ToString()) < 0;
    });
}

void UEDInventoryBlueprintLibrary::BuildCraftTreePaths(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, TArray<FEDCraftTreePath>& OutPaths, bool& bOutCyclePruned, int32 MaxDepth)
{
    OutPaths.Reset();
    bOutCyclePruned = false;

    if (!InventoryComponent || !ResultItemId.IsValid())
    {
        return;
    }

    TArray<FCraftRecipeViewRow> Recipes;
    GatherCraftRecipes(InventoryComponent, Recipes);

    const int32 ClampedMaxDepth = FMath::Clamp(MaxDepth, 1, 8);

    TArray<const FCraftRecipeViewRow*> RootRecipes;
    for (const FCraftRecipeViewRow& Recipe : Recipes)
    {
        if (Recipe.ResultItemId == ResultItemId)
        {
            RootRecipes.Add(&Recipe);
        }
    }

    if (RootRecipes.Num() <= 0)
    {
        FEDCraftTreePath Path;
        FEDCraftTreePathNode Node;
        Node.ItemId = ResultItemId;
        Node.Quantity = 1;
        Node.Depth = 0;
        Path.Nodes.Add(MoveTemp(Node));
        OutPaths.Add(MoveTemp(Path));
        return;
    }

    for (const FCraftRecipeViewRow* RootRecipe : RootRecipes)
    {
        if (!RootRecipe)
        {
            continue;
        }

        TSet<FPrimaryAssetId> ActivePath;
        ActivePath.Add(ResultItemId);

        TArray<FEDCraftTreePathNode> CurrentPath;
        BuildPathRecursive(
            ResultItemId,
            1,
            0,
            RootRecipe->RowId,
            RootRecipe->RecipeId,
            RootRecipe->RowId,
            RootRecipe->RecipeId,
            Recipes,
            ActivePath,
            ClampedMaxDepth,
            CurrentPath,
            OutPaths,
            bOutCyclePruned);
    }
}

void UEDInventoryBlueprintLibrary::GetCraftRecipeViewDataList(
    UEDInventoryComponent* InventoryComponent,
    TArray<FEDCraftRecipeViewData>& OutRecipes,
    bool bOnlyCraftable,
    EEDCraftableRecipeSortOption SortOption,
    bool bDescending)
{
    // 제작 UI 우측 리스트용 전체 레시피 데이터를 만듦
    // 필요하면 현재 제작 가능한 레시피만 필터링 가능
    OutRecipes.Reset();

    if (!InventoryComponent)
    {
        return;
    }

    TArray<FCraftRecipeViewRow> Recipes;
    GatherCraftRecipes(InventoryComponent, Recipes);

    for (const FCraftRecipeViewRow& Recipe : Recipes)
    {
        FEDCraftRecipeViewData RecipeView;
        if (!BuildCraftRecipeViewData(InventoryComponent, Recipe, RecipeView))
        {
            continue;
        }

        if (bOnlyCraftable && !RecipeView.bCanCraft)
        {
            continue;
        }

        OutRecipes.Add(MoveTemp(RecipeView));
    }

    OutRecipes.Sort([SortOption, bDescending](const FEDCraftRecipeViewData& A, const FEDCraftRecipeViewData& B)
    {
        const int32 Compare = CompareCraftRecipeViewData(A, B, SortOption);
        return bDescending ? (Compare > 0) : (Compare < 0);
    });
}

bool UEDInventoryBlueprintLibrary::GetCraftRecipeViewDataByRowId(
    UEDInventoryComponent* InventoryComponent,
    FName RecipeRowId,
    FEDCraftRecipeViewData& OutRecipe)
{
    // 선택된 레시피 1개를 상세 패널에 표시할 때 사용
    OutRecipe = FEDCraftRecipeViewData();

    if (!InventoryComponent || RecipeRowId.IsNone())
    {
        return false;
    }

    TArray<FCraftRecipeViewRow> Recipes;
    GatherCraftRecipes(InventoryComponent, Recipes);

    for (const FCraftRecipeViewRow& Recipe : Recipes)
    {
        if (Recipe.RowId != RecipeRowId)
        {
            continue;
        }

        return BuildCraftRecipeViewData(InventoryComponent, Recipe, OutRecipe);
    }

    return false;
}
