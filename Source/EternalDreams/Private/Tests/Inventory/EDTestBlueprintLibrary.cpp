#include "Tests/Inventory/EDTestBlueprintLibrary.h"

#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/System/EDInventoryCraftService.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Item/Data/EDItemDataRows.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"

namespace
{
FString ResolveItemName(const FPrimaryAssetId& ItemId)
{
    if (!ItemId.IsValid())
    {
        return TEXT("None");
    }

    UAssetManager& AssetManager = UAssetManager::Get();
    if (UObject* AssetObject = AssetManager.GetPrimaryAssetObject(ItemId))
    {
        if (const UEDInventoryItemDataAsset* ItemData = Cast<UEDInventoryItemDataAsset>(AssetObject))
        {
            if (!ItemData->DisplayName.IsEmpty())
            {
                return ItemData->DisplayName.ToString();
            }
        }
    }

    return ItemId.PrimaryAssetName.ToString();
}

FString FormatItemLine(const FEDInventoryItemHandle& Item)
{
    return FString::Printf(TEXT("%s x%d"), *ResolveItemName(Item.ItemId), Item.Quantity);
}

void AppendEquipmentLine(TArray<FString>& OutLines, const TCHAR* SlotLabel, const FEDEquipmentSlotData& EquipmentSlot)
{
    if (!EquipmentSlot.EquippedItem.IsValid())
    {
        return;
    }

    OutLines.Add(FString::Printf(TEXT("%s: %s"), SlotLabel, *FormatItemLine(EquipmentSlot.EquippedItem)));
}

bool PassesAuthorityFilter(const UEDInventoryComponent* InventoryComponent, bool bOnlyAuthority)
{
    if (!bOnlyAuthority)
    {
        return true;
    }

    const AActor* OwnerActor = IsValid(InventoryComponent) ? InventoryComponent->GetOwner() : nullptr;
    return IsValid(OwnerActor) && OwnerActor->HasAuthority();
}

const TCHAR* ToRarityLabel(EEDItemRarity Rarity)
{
    switch (Rarity)
    {
    case EEDItemRarity::Normal:
        return TEXT("Normal");
    case EEDItemRarity::Rare:
        return TEXT("Rare");
    case EEDItemRarity::Epic:
        return TEXT("Epic");
    case EEDItemRarity::Legendary:
        return TEXT("Legendary");
    case EEDItemRarity::Unique:
        return TEXT("Unique");
    default:
        return TEXT("Unknown");
    }
}

const TCHAR* ToItemTypeLabel(EEDInventoryItemType ItemType)
{
    switch (ItemType)
    {
    case EEDInventoryItemType::Equippable:
        return TEXT("Equippable");
    case EEDInventoryItemType::Consumable:
        return TEXT("Consumable");
    case EEDInventoryItemType::Ingredient:
        return TEXT("Ingredient");
    case EEDInventoryItemType::Skill:
        return TEXT("Skill");
    default:
        return TEXT("Unknown");
    }
}

const TCHAR* ToEquippableTypeLabel(EEDEquippableType EquippableType)
{
    switch (EquippableType)
    {
    case EEDEquippableType::None:
        return TEXT("None");
    case EEDEquippableType::Weapon:
        return TEXT("Weapon");
    case EEDEquippableType::TopArmor:
        return TEXT("TopArmor");
    case EEDEquippableType::BottomArmor:
        return TEXT("BottomArmor");
    default:
        return TEXT("Unknown");
    }
}

FString FormatTagContainer(const FGameplayTagContainer& Tags)
{
    TArray<FString> TagStrings;
    TagStrings.Reserve(Tags.Num());

    for (const FGameplayTag& Tag : Tags)
    {
        if (Tag.IsValid())
        {
            TagStrings.Add(Tag.ToString());
        }
    }

    TagStrings.Sort();
    return TagStrings.Num() > 0 ? FString::Join(TagStrings, TEXT(",")) : TEXT("-");
}

FString FormatCraftableLine(const FEDCraftableRecipeEntry& Entry)
{
    const FString ItemName = Entry.ResultItemName.IsEmpty() ? Entry.ResultItemId.PrimaryAssetName.ToString() : Entry.ResultItemName.ToString();
    return FString::Printf(TEXT("%s -> %s (%s)"), *Entry.RowId.ToString(), *ItemName, ToRarityLabel(Entry.ResultRarity));
}

FString FormatDictionaryEntryLine(const FEDItemDictionaryEntry& Entry)
{
    const FString DisplayName = Entry.DisplayName.IsEmpty() ? Entry.ItemId.PrimaryAssetName.ToString() : Entry.DisplayName.ToString();

    return FString::Printf(
        TEXT("%s | Id=%s | Type=%s | Equip=%s | Rarity=%s | Stack=%d | Price=%d | Tags=%s"),
        *DisplayName,
        *Entry.ItemId.ToString(),
        ToItemTypeLabel(Entry.ItemType),
        ToEquippableTypeLabel(Entry.EquippableType),
        ToRarityLabel(Entry.Rarity),
        Entry.MaxStack,
        Entry.Price,
        *FormatTagContainer(Entry.ItemTags));
}

bool BuildInventoryBlock(const UEDInventoryComponent* InventoryComponent, bool bOnlyAuthority, FString& OutBlock)
{
    OutBlock.Empty();

    if (!IsValid(InventoryComponent) || !PassesAuthorityFilter(InventoryComponent, bOnlyAuthority))
    {
        return false;
    }

    const AActor* OwnerActor = InventoryComponent->GetOwner();
    if (!IsValid(OwnerActor))
    {
        return false;
    }

    TArray<FString> EquipmentLines;
    if (InventoryComponent->bUseEquipmentSlots)
    {
        AppendEquipmentLine(EquipmentLines, TEXT("weapon"), InventoryComponent->WeaponSlot);
        AppendEquipmentLine(EquipmentLines, TEXT("toparmor"), InventoryComponent->TopArmorSlot);
        AppendEquipmentLine(EquipmentLines, TEXT("bottomarmor"), InventoryComponent->BottomArmorSlot);
    }

    TArray<FString> InventoryLines;
    for (int32 SlotIndex = 0; SlotIndex < InventoryComponent->InventorySlots.Num(); ++SlotIndex)
    {
        const FEDInventorySlotData& SlotData = InventoryComponent->InventorySlots[SlotIndex];
        if (!SlotData.Item.IsValid())
        {
            continue;
        }

        InventoryLines.Add(FString::Printf(TEXT("slot%d: %s"), SlotIndex, *FormatItemLine(SlotData.Item)));
    }

    TArray<FString> CraftableLines;
    for (const FEDCraftableRecipeEntry& Craftable : InventoryComponent->CachedCraftableRecipes)
    {
        CraftableLines.Add(FormatCraftableLine(Craftable));
    }

    if (EquipmentLines.Num() == 0 && InventoryLines.Num() == 0 && CraftableLines.Num() == 0)
    {
        return false;
    }

    OutBlock = OwnerActor->GetName();

    if (EquipmentLines.Num() > 0)
    {
        OutBlock += TEXT(" - ");
        OutBlock += FString::Join(EquipmentLines, TEXT(" / "));
    }

    if (InventoryLines.Num() > 0)
    {
        OutBlock += TEXT("\nInventory:\n");
        OutBlock += FString::Join(InventoryLines, TEXT("\n"));
    }

    if (CraftableLines.Num() > 0)
    {
        OutBlock += TEXT("\nCraftable:\n");
        OutBlock += FString::Join(CraftableLines, TEXT("\n"));
    }

    return true;
}

FString FormatCraftIngredientLine(const FEDCraftingIngredientRow& Ingredient)
{
    return FString::Printf(TEXT("%s x%d"), *ResolveItemName(Ingredient.ItemId), Ingredient.Quantity);
}

FString FormatCraftRecipeLine(const FName& RowName, const FEDCraftingRecipeRow& Recipe, bool bCanCraftNow)
{
    TArray<FString> IngredientTexts;
    IngredientTexts.Reserve(Recipe.Ingredients.Num());

    for (const FEDCraftingIngredientRow& Ingredient : Recipe.Ingredients)
    {
        IngredientTexts.Add(FormatCraftIngredientLine(Ingredient));
    }

    const FString IngredientsText = IngredientTexts.Num() > 0
        ? FString::Join(IngredientTexts, TEXT(", "))
        : TEXT("(No Ingredients)");

    const FString RecipeIdText = Recipe.RecipeId.IsNone() ? TEXT("None") : Recipe.RecipeId.ToString();
    const FString CraftableText = bCanCraftNow ? TEXT("CanCraftNow") : TEXT("MissingIngredients");

    return FString::Printf(
        TEXT("[%s] Row=%s | RecipeId=%s | Result=%s x%d | Ingredients=%s"),
        *CraftableText,
        *RowName.ToString(),
        *RecipeIdText,
        *ResolveItemName(Recipe.ResultItemId),
        Recipe.ResultQuantity,
        *IngredientsText);
}

FString FormatCraftTreeNodeLine(const FEDCraftTreeFlatNode& Node)
{
    return FString::Printf(
        TEXT("Depth=%d | Node=%d | Parent=%d | Item=%s x%d | ViaRow=%s | ViaRecipe=%s | HasRecipe=%s | Pruned=%s"),
        Node.Depth,
        Node.NodeId,
        Node.ParentNodeId,
        *ResolveItemName(Node.ItemId),
        Node.Quantity,
        *Node.RowId.ToString(),
        *Node.RecipeId.ToString(),
        Node.bIsCraftable ? TEXT("Y") : TEXT("N"),
        Node.bCyclePruned ? TEXT("Y") : TEXT("N"));
}
}

AActor* UEDTestBlueprintLibrary::GetClosestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf)
{
    if (!IsValid(SourceActor) || !*ActorClass)
    {
        return nullptr;
    }

    UWorld* World = SourceActor->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Candidates);

    const FVector SourceLocation = SourceActor->GetActorLocation();
    AActor* ClosestActor = nullptr;
    float ClosestDistanceSq = TNumericLimits<float>::Max();

    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate))
        {
            continue;
        }

        if (!bIncludeSelf && Candidate == SourceActor)
        {
            continue;
        }

        const float DistanceSq = FVector::DistSquared(SourceLocation, Candidate->GetActorLocation());
        if (DistanceSq < ClosestDistanceSq)
        {
            ClosestDistanceSq = DistanceSq;
            ClosestActor = Candidate;
        }
    }

    return ClosestActor;
}

AActor* UEDTestBlueprintLibrary::GetFarthestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf)
{
    if (!IsValid(SourceActor) || !*ActorClass)
    {
        return nullptr;
    }

    UWorld* World = SourceActor->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Candidates);

    const FVector SourceLocation = SourceActor->GetActorLocation();
    AActor* FarthestActor = nullptr;
    float FarthestDistanceSq = -1.0f;

    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate))
        {
            continue;
        }

        if (!bIncludeSelf && Candidate == SourceActor)
        {
            continue;
        }

        const float DistanceSq = FVector::DistSquared(SourceLocation, Candidate->GetActorLocation());
        if (DistanceSq > FarthestDistanceSq)
        {
            FarthestDistanceSq = DistanceSq;
            FarthestActor = Candidate;
        }
    }

    return FarthestActor;
}

FString UEDTestBlueprintLibrary::BuildAllInventoryDebugText(const UObject* WorldContextObject, bool bOnlyAuthority)
{
    TArray<UEDInventoryComponent*> InventoryComponents;
    GetAllWorldInventoryComponents(WorldContextObject, InventoryComponents, bOnlyAuthority);

    if (!IsValid(WorldContextObject))
    {
        return TEXT("InventoryDebug: Invalid WorldContextObject");
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return TEXT("InventoryDebug: World is null");
    }

    return BuildInventoryDebugTextFromArray(InventoryComponents, bOnlyAuthority);
}

FString UEDTestBlueprintLibrary::BuildInventoryDebugTextFromArray(const TArray<UEDInventoryComponent*>& InventoryComponents, bool bOnlyAuthority)
{
    TArray<FString> ActorBlocks;

    for (const UEDInventoryComponent* InventoryComponent : InventoryComponents)
    {
        FString Block;
        if (BuildInventoryBlock(InventoryComponent, bOnlyAuthority, Block))
        {
            ActorBlocks.Add(MoveTemp(Block));
        }
    }

    if (ActorBlocks.Num() == 0)
    {
        return bOnlyAuthority
            ? TEXT("InventoryDebug: No non-empty authoritative inventory/equipment found.")
            : TEXT("InventoryDebug: No non-empty inventory/equipment found.");
    }

    ActorBlocks.Sort();
    return FString::Join(ActorBlocks, TEXT("\n\n"));
}

void UEDTestBlueprintLibrary::GetAllWorldInventoryComponents(const UObject* WorldContextObject, TArray<UEDInventoryComponent*>& OutInventoryComponents, bool bOnlyAuthority)
{
    OutInventoryComponents.Reset();

    if (!IsValid(WorldContextObject))
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return;
    }

    for (TObjectIterator<UEDInventoryComponent> It; It; ++It)
    {
        UEDInventoryComponent* InventoryComponent = *It;
        if (!IsValid(InventoryComponent) || InventoryComponent->GetWorld() != World)
        {
            continue;
        }

        if (!PassesAuthorityFilter(InventoryComponent, bOnlyAuthority))
        {
            continue;
        }

        OutInventoryComponents.Add(InventoryComponent);
    }
}

void UEDTestBlueprintLibrary::PrintAllInventoryDebugText(const UObject* WorldContextObject, float Duration, bool bOnlyAuthority)
{
    const FString DebugText = BuildAllInventoryDebugText(WorldContextObject, bOnlyAuthority);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(777002, Duration, FColor::Cyan, DebugText);
    }
}

UTexture2D* UEDTestBlueprintLibrary::GetItemIconByAssetId(FPrimaryAssetId ItemId)
{
    if (!ItemId.IsValid())
    {
        return nullptr;
    }

    UAssetManager& AssetManager = UAssetManager::Get();
    UObject* AssetObject = AssetManager.GetPrimaryAssetObject(ItemId);
    const UEDInventoryItemDataAsset* ItemData = Cast<UEDInventoryItemDataAsset>(AssetObject);
    return IsValid(ItemData) ? ItemData->IconTexture.Get() : nullptr;
}

FString UEDTestBlueprintLibrary::BuildItemDictionaryDebugStringByTags(const FGameplayTagContainer& FilterTags, EEDItemDictionarySortOption SortOption, bool bDescending)
{
    TArray<FEDItemDictionaryEntry> Items;
    UEDInventoryBlueprintLibrary::GetItemDictionaryByTags(FilterTags, Items, SortOption, bDescending);

    if (Items.Num() <= 0)
    {
        return TEXT("ItemDictionary: No items matched the filter tags.");
    }

    TArray<FString> Lines;
    Lines.Reserve(Items.Num() + 1);
    Lines.Add(FString::Printf(TEXT("ItemDictionary: %d item(s)"), Items.Num()));

    for (const FEDItemDictionaryEntry& Entry : Items)
    {
        Lines.Add(FormatDictionaryEntryLine(Entry));
    }

    return FString::Join(Lines, TEXT("\n"));
}

FText UEDTestBlueprintLibrary::BuildItemDictionaryDebugTextByTags(const FGameplayTagContainer& FilterTags, EEDItemDictionarySortOption SortOption, bool bDescending)
{
    return FText::FromString(BuildItemDictionaryDebugStringByTags(FilterTags, SortOption, bDescending));
}

FString UEDTestBlueprintLibrary::BuildCraftRecipeDebugString(UEDInventoryComponent* InventoryComponent, bool bOnlyCraftable)
{
    if (!IsValid(InventoryComponent))
    {
        return TEXT("CraftRecipeDebug: InventoryComponent is invalid.");
    }

    TArray<UDataTable*> CraftTables;
    InventoryComponent->GetAllCraftingRecipeTables(CraftTables);
    if (CraftTables.Num() <= 0)
    {
        return TEXT("CraftRecipeDebug: No crafting recipe tables are assigned.");
    }

    TArray<FString> Lines;
    const FString OwnerName = IsValid(InventoryComponent->GetOwner()) ? InventoryComponent->GetOwner()->GetName() : TEXT("None");
    Lines.Add(FString::Printf(TEXT("CraftRecipeDebug: Owner=%s, Tables=%d"), *OwnerName, CraftTables.Num()));

    int32 TotalRecipeCount = 0;
    int32 VisibleRecipeCount = 0;

    for (UDataTable* Table : CraftTables)
    {
        if (!IsValid(Table))
        {
            Lines.Add(TEXT("- Table: <Invalid>"));
            continue;
        }

        const FString TableName = Table->GetName();
        TArray<FName> RowNames = Table->GetRowNames();
        RowNames.Sort(FNameLexicalLess());

        int32 TableVisibleCount = 0;
        Lines.Add(FString::Printf(TEXT("- Table: %s (%d row(s))"), *TableName, RowNames.Num()));

        for (const FName& RowName : RowNames)
        {
            ++TotalRecipeCount;

            const FEDCraftingRecipeRow* RecipeRow = Table->FindRow<FEDCraftingRecipeRow>(RowName, TEXT("BuildCraftRecipeDebugString"));
            if (!RecipeRow)
            {
                continue;
            }

            // Check craftability against this exact row to avoid duplicate RowName collisions across multiple tables.
            const bool bCanCraftNow = FEDInventoryCraftService::CanCraftRecipe(InventoryComponent, *RecipeRow, nullptr);
            if (bOnlyCraftable && !bCanCraftNow)
            {
                continue;
            }

            ++VisibleRecipeCount;
            ++TableVisibleCount;
            Lines.Add(FString::Printf(TEXT("  %s"), *FormatCraftRecipeLine(RowName, *RecipeRow, bCanCraftNow)));
        }

        if (TableVisibleCount == 0)
        {
            Lines.Add(TEXT("  (No recipe matched current filter)"));
        }
    }

    Lines.Add(FString::Printf(TEXT("Summary: Visible=%d / Total=%d"), VisibleRecipeCount, TotalRecipeCount));
    return FString::Join(Lines, TEXT("\n"));
}

FText UEDTestBlueprintLibrary::BuildCraftRecipeDebugText(UEDInventoryComponent* InventoryComponent, bool bOnlyCraftable)
{
    return FText::FromString(BuildCraftRecipeDebugString(InventoryComponent, bOnlyCraftable));
}

void UEDTestBlueprintLibrary::PrintCraftRecipeDebugText(UEDInventoryComponent* InventoryComponent, float Duration, bool bOnlyCraftable)
{
    const FString DebugText = BuildCraftRecipeDebugString(InventoryComponent, bOnlyCraftable);
    const float VisibleDuration = Duration > 0.0f ? Duration : 8.0f;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(777003, VisibleDuration, FColor::Orange, DebugText);
    }

    UE_LOG(LogTemp, Log, TEXT("%s"), *DebugText);
}

FString UEDTestBlueprintLibrary::BuildCraftTreeDebugStringByItemId(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, int32 MaxDepth)
{
    if (!IsValid(InventoryComponent))
    {
        return TEXT("CraftTreeDebug: InventoryComponent is invalid.");
    }

    if (!ResultItemId.IsValid())
    {
        return TEXT("CraftTreeDebug: ResultItemId is invalid.");
    }

    const int32 SafeMaxDepth = FMath::Clamp(MaxDepth, 1, 32);

    TArray<FEDCraftTreeFlatNode> FlatNodes;
    bool bCyclePruned = false;
    UEDInventoryBlueprintLibrary::BuildCraftTreeFlat(InventoryComponent, ResultItemId, FlatNodes, bCyclePruned, SafeMaxDepth);

    if (FlatNodes.Num() <= 0)
    {
        return FString::Printf(TEXT("CraftTreeDebug: No recipe tree found for %s."), *ResolveItemName(ResultItemId));
    }

    TArray<FString> Lines;
    Lines.Reserve(FlatNodes.Num() + 2);
    Lines.Add(FString::Printf(TEXT("CraftTreeDebug: Root=%s, NodeCount=%d, MaxDepth=%d, CyclePruned=%s"),
        *ResolveItemName(ResultItemId),
        FlatNodes.Num(),
        SafeMaxDepth,
        bCyclePruned ? TEXT("Y") : TEXT("N")));

    for (const FEDCraftTreeFlatNode& Node : FlatNodes)
    {
        Lines.Add(FormatCraftTreeNodeLine(Node));
    }

    return FString::Join(Lines, TEXT("\n"));
}

FText UEDTestBlueprintLibrary::BuildCraftTreeDebugTextByItemId(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, int32 MaxDepth)
{
    return FText::FromString(BuildCraftTreeDebugStringByItemId(InventoryComponent, ResultItemId, MaxDepth));
}

void UEDTestBlueprintLibrary::PrintCraftTreeDebugTextByItemId(UEDInventoryComponent* InventoryComponent, FPrimaryAssetId ResultItemId, float Duration, int32 MaxDepth)
{
    const FString DebugText = BuildCraftTreeDebugStringByItemId(InventoryComponent, ResultItemId, MaxDepth);
    const float VisibleDuration = Duration > 0.0f ? Duration : 8.0f;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(777004, VisibleDuration, FColor::Green, DebugText);
    }

    UE_LOG(LogTemp, Log, TEXT("%s"), *DebugText);
}
