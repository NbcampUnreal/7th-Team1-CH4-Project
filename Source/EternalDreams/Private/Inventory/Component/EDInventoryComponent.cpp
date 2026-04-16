#include "Inventory/Component/EDInventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/AssetManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Inventory/GAS/EDInventoryGASBridge.h"
#include "Inventory/System/EDInventoryCraftService.h"
#include "Inventory/System/EDInventoryEquipmentService.h"
#include "Inventory/System/EDInventoryTransferService.h"
#include "Inventory/System/EDInventoryValidationService.h"
#include "Inventory/World/EDDroppedItemActor.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Item/Data/EDItemDataRows.h"
#include "Net/UnrealNetwork.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveItemData_Component(const FPrimaryAssetId& ItemId)
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

int32 GetItemMaxStack_Component(const FPrimaryAssetId& ItemId)
{
    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(ItemId);
    return ItemData ? FMath::Max(1, ItemData->MaxStack) : 1;
}

int32 GetReceivableCapacity_Component(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    if (!InventoryComponent || !ItemId.IsValid())
    {
        return 0;
    }

    const int32 MaxStack = GetItemMaxStack_Component(ItemId);
    int32 Capacity = 0;

    for (const FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Slot.IsEmpty())
        {
            Capacity += MaxStack;
            continue;
        }

        if (Slot.Item.ItemId == ItemId)
        {
            Capacity += FMath::Max(0, MaxStack - Slot.Item.Quantity);
        }
    }

    return Capacity;
}

bool AddItemAuto_Component(UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity)
{
    if (!InventoryComponent || !ItemId.IsValid() || Quantity <= 0)
    {
        return false;
    }

    if (GetReceivableCapacity_Component(InventoryComponent, ItemId) < Quantity)
    {
        return false;
    }

    const int32 MaxStack = GetItemMaxStack_Component(ItemId);
    int32 Remaining = Quantity;

    for (FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Remaining <= 0)
        {
            break;
        }

        if (!Slot.IsEmpty() && Slot.Item.ItemId == ItemId)
        {
            const int32 Space = FMath::Max(0, MaxStack - Slot.Item.Quantity);
            const int32 AddAmount = FMath::Min(Remaining, Space);
            if (AddAmount > 0)
            {
                Slot.Item.Quantity += AddAmount;
                Remaining -= AddAmount;
            }
        }
    }

    for (FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Remaining <= 0)
        {
            break;
        }

        if (Slot.IsEmpty())
        {
            const int32 AddAmount = FMath::Min(Remaining, MaxStack);
            Slot.Item.ItemId = ItemId;
            Slot.Item.Quantity = AddAmount;
            Remaining -= AddAmount;
        }
    }

    return Remaining == 0;
}

bool AddItemToSlot_Component(UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure* OutFailure)
{
    if (!InventoryComponent)
    {
        if (OutFailure)
        {
            *OutFailure = EEDInventoryActionFailure::InvalidInventory;
        }
        return false;
    }

    if (!ItemId.IsValid())
    {
        if (OutFailure)
        {
            *OutFailure = EEDInventoryActionFailure::MissingData;
        }
        return false;
    }

    if (!InventoryComponent->InventorySlots.IsValidIndex(SlotIndex))
    {
        if (OutFailure)
        {
            *OutFailure = EEDInventoryActionFailure::InvalidSlot;
        }
        return false;
    }

    if (Quantity <= 0)
    {
        if (OutFailure)
        {
            *OutFailure = EEDInventoryActionFailure::InvalidQuantity;
        }
        return false;
    }

    FEDInventorySlotData& Slot = InventoryComponent->InventorySlots[SlotIndex];
    const int32 MaxStack = GetItemMaxStack_Component(ItemId);

    if (Slot.IsEmpty())
    {
        if (Quantity > MaxStack)
        {
            if (OutFailure)
            {
                *OutFailure = EEDInventoryActionFailure::StackLimit;
            }
            return false;
        }

        Slot.Item.ItemId = ItemId;
        Slot.Item.Quantity = Quantity;
        return true;
    }

    if (Slot.Item.ItemId != ItemId)
    {
        if (OutFailure)
        {
            *OutFailure = EEDInventoryActionFailure::SlotConflict;
        }
        return false;
    }

    const int32 Space = FMath::Max(0, MaxStack - Slot.Item.Quantity);
    if (Quantity > Space)
    {
        if (OutFailure)
        {
            *OutFailure = EEDInventoryActionFailure::StackLimit;
        }
        return false;
    }

    Slot.Item.Quantity += Quantity;
    return true;
}

const FEDItemSpawnRow* PickLootRowByWeight_Component(const TArray<const FEDItemSpawnRow*>& CandidateRows, FRandomStream& RandomStream)
{
    float TotalWeight = 0.0f;
    for (const FEDItemSpawnRow* Row : CandidateRows)
    {
        if (Row)
        {
            TotalWeight += FMath::Max(0.0f, Row->Weight);
        }
    }

    if (TotalWeight <= 0.0f)
    {
        return nullptr;
    }

    const float Pick = RandomStream.FRandRange(0.0f, TotalWeight);
    float AccWeight = 0.0f;
    for (const FEDItemSpawnRow* Row : CandidateRows)
    {
        if (!Row)
        {
            continue;
        }

        AccWeight += FMath::Max(0.0f, Row->Weight);
        if (Pick <= AccWeight)
        {
            return Row;
        }
    }

    return CandidateRows.Num() > 0 ? CandidateRows.Last() : nullptr;
}

void AddLooseTags(UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags)
{
    if (!ASC)
    {
        return;
    }

    TArray<FGameplayTag> TagArray;
    Tags.GetGameplayTagArray(TagArray);
    for (const FGameplayTag& Tag : TagArray)
    {
        if (Tag.IsValid())
        {
            ASC->AddLooseGameplayTag(Tag);
        }
    }
}

void RemoveLooseTags(UAbilitySystemComponent* ASC, const FGameplayTagContainer& Tags)
{
    if (!ASC)
    {
        return;
    }

    TArray<FGameplayTag> TagArray;
    Tags.GetGameplayTagArray(TagArray);
    for (const FGameplayTag& Tag : TagArray)
    {
        if (Tag.IsValid())
        {
            ASC->RemoveLooseGameplayTag(Tag);
        }
    }
}

FRandomStream BuildRandomStreamFromOptionalSeed(const int32 Seed)
{
    FRandomStream RandomStream;
    if (Seed != 0)
    {
        RandomStream.Initialize(Seed);
    }
    else
    {
        RandomStream.GenerateNewSeed();
    }
    return RandomStream;
}

UEDInventoryComponent* ResolveInventoryComponentFromActor_Component(AActor* Actor)
{
    return Actor ? Actor->FindComponentByClass<UEDInventoryComponent>() : nullptr;
}

EEDItemRarity ResolveItemRarity_Component(const FPrimaryAssetId& ItemId)
{
    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(ItemId);
    return ItemData ? ItemData->Rarity : EEDItemRarity::Normal;
}

template<typename T>
void ShuffleArray_Component(TArray<T>& Array, FRandomStream& RandomStream)
{
    for (int32 Index = Array.Num() - 1; Index > 0; --Index)
    {
        const int32 SwapIndex = RandomStream.RandRange(0, Index);
        if (SwapIndex != Index)
        {
            Array.Swap(Index, SwapIndex);
        }
    }
}

int32 GetReceivableCapacityForTargets_Component(const TArray<UEDInventoryComponent*>& Targets, const FPrimaryAssetId& ItemId)
{
    int32 TotalCapacity = 0;
    for (const UEDInventoryComponent* Target : Targets)
    {
        TotalCapacity += GetReceivableCapacity_Component(Target, ItemId);
    }
    return TotalCapacity;
}

void SetFailure_Component(EEDInventoryActionFailure* OutFailure, EEDInventoryActionFailure Failure)
{
    if (OutFailure)
    {
        *OutFailure = Failure;
    }
}

bool PrecheckTransferAuto_Component(const UEDInventoryComponent* FromInventory, const UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure* OutFailure)
{
    SetFailure_Component(OutFailure, EEDInventoryActionFailure::None);

    if (!FromInventory || !ToInventory)
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::InvalidInventory);
        return false;
    }

    if (!FromInventory->InventorySlots.IsValidIndex(FromSlotIndex))
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::InvalidSlot);
        return false;
    }

    const FEDInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::EmptySlot);
        return false;
    }

    const int32 MoveQuantity = FMath::Min(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::InvalidQuantity);
        return false;
    }

    if (GetReceivableCapacity_Component(ToInventory, SourceSlot.Item.ItemId) <= 0)
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::NoSpace);
        return false;
    }

    return true;
}

bool PrecheckTransferToSlot_Component(const UEDInventoryComponent* FromInventory, const UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure* OutFailure)
{
    SetFailure_Component(OutFailure, EEDInventoryActionFailure::None);

    if (!FromInventory || !ToInventory)
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::InvalidInventory);
        return false;
    }

    if (FromInventory == ToInventory && FromSlotIndex == ToSlotIndex)
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::SlotConflict);
        return false;
    }

    if (!FromInventory->InventorySlots.IsValidIndex(FromSlotIndex) || !ToInventory->InventorySlots.IsValidIndex(ToSlotIndex))
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::InvalidSlot);
        return false;
    }

    const FEDInventorySlotData& SourceSlot = FromInventory->InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::EmptySlot);
        return false;
    }

    const int32 MoveQuantity = FMath::Min(Quantity, SourceSlot.Item.Quantity);
    if (MoveQuantity <= 0)
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::InvalidQuantity);
        return false;
    }

    const FEDInventorySlotData& DestinationSlot = ToInventory->InventorySlots[ToSlotIndex];
    if (DestinationSlot.IsEmpty())
    {
        return true;
    }

    const int32 MaxStack = GetItemMaxStack_Component(SourceSlot.Item.ItemId);
    if (DestinationSlot.Item.ItemId == SourceSlot.Item.ItemId)
    {
        const int32 SpaceLeft = FMath::Max(0, MaxStack - DestinationSlot.Item.Quantity);
        if (SpaceLeft <= 0)
        {
            SetFailure_Component(OutFailure, EEDInventoryActionFailure::StackLimit);
            return false;
        }
        return true;
    }

    if (MoveQuantity != SourceSlot.Item.Quantity)
    {
        SetFailure_Component(OutFailure, EEDInventoryActionFailure::SlotConflict);
        return false;
    }

    return true;
}

bool TryFindRecipeById_Component(const UEDInventoryComponent* InventoryComponent, FName RecipeId, FEDCraftingRecipeRow& OutRecipe)
{
    if (!InventoryComponent || RecipeId.IsNone())
    {
        return false;
    }

    for (UDataTable* RecipeTable : InventoryComponent->CraftingRecipeTables)
    {
        if (!RecipeTable)
        {
            continue;
        }

        TArray<FName> RowNames = RecipeTable->GetRowNames();
        for (const FName RowName : RowNames)
        {
            const FEDCraftingRecipeRow* Row = RecipeTable->FindRow<FEDCraftingRecipeRow>(RowName, TEXT("TryFindRecipeById"));
            if (Row && Row->RecipeId == RecipeId)
            {
                OutRecipe = *Row;
                return true;
            }
        }
    }

    return false;
}

bool IsRaritySortingEnabled_Component(const UEDInventoryComponent* InventoryComponent)
{
    return InventoryComponent
        && InventoryComponent->DistributionRaritySecondaryMode == EEDInventoryRaritySecondarySplitMode::RaritySorting;
}

int32 GetRarityLoadScore_Component(const UEDInventoryComponent* Target, EEDItemRarity Rarity, EEDInventorySplitMode SplitMode)
{
    if (!Target)
    {
        return TNumericLimits<int32>::Max();
    }

    if (SplitMode == EEDInventorySplitMode::ByType)
    {
        TSet<FPrimaryAssetId> UniqueItemIds;
        for (const FEDInventorySlotData& Slot : Target->InventorySlots)
        {
            if (Slot.IsEmpty() || !Slot.Item.ItemId.IsValid())
            {
                continue;
            }

            if (ResolveItemRarity_Component(Slot.Item.ItemId) == Rarity)
            {
                UniqueItemIds.Add(Slot.Item.ItemId);
            }
        }
        return UniqueItemIds.Num();
    }

    int32 TotalQuantity = 0;
    for (const FEDInventorySlotData& Slot : Target->InventorySlots)
    {
        if (Slot.IsEmpty() || !Slot.Item.ItemId.IsValid())
        {
            continue;
        }

        if (ResolveItemRarity_Component(Slot.Item.ItemId) == Rarity)
        {
            TotalQuantity += FMath::Max(0, Slot.Item.Quantity);
        }
    }
    return TotalQuantity;
}

UEDInventoryComponent* PickCandidateWithLowestRarityLoad_Component(
    const TArray<UEDInventoryComponent*>& Candidates,
    EEDItemRarity Rarity,
    EEDInventorySplitMode SplitMode,
    FRandomStream& RandomStream)
{
    if (Candidates.Num() == 0)
    {
        return nullptr;
    }

    int32 MinScore = TNumericLimits<int32>::Max();
    TArray<UEDInventoryComponent*> BestCandidates;

    for (UEDInventoryComponent* Candidate : Candidates)
    {
        if (!Candidate)
        {
            continue;
        }

        const int32 Score = GetRarityLoadScore_Component(Candidate, Rarity, SplitMode);
        if (Score < MinScore)
        {
            MinScore = Score;
            BestCandidates.Reset();
            BestCandidates.Add(Candidate);
        }
        else if (Score == MinScore)
        {
            BestCandidates.Add(Candidate);
        }
    }

    if (BestCandidates.Num() == 0)
    {
        return nullptr;
    }

    return BestCandidates[RandomStream.RandRange(0, BestCandidates.Num() - 1)];
}

int32 ResolveDistributionMoveQuantity_Component(
    int32 AvailableQuantity,
    int32 Capacity,
    EEDInventoryStackPolicy StackPolicy,
    FRandomStream& RandomStream)
{
    const int32 MaxMovable = FMath::Min(AvailableQuantity, Capacity);
    if (MaxMovable <= 0)
    {
        return 0;
    }

    switch (StackPolicy)
    {
    case EEDInventoryStackPolicy::SplitIfPossible:
        return 1;
    case EEDInventoryStackPolicy::UnionAsPossible:
        return MaxMovable;
    case EEDInventoryStackPolicy::Randomize:
    default:
        return RandomStream.RandRange(1, MaxMovable);
    }
}
}

UEDInventoryComponent::UEDInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    DroppedItemActorClass = AEDDroppedItemActor::StaticClass();

    WeaponSlot.SlotType = EEDEquippableType::Weapon;
    TopArmorSlot.SlotType = EEDEquippableType::TopArmor;
    BottomArmorSlot.SlotType = EEDEquippableType::BottomArmor;
}

void UEDInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    OnInventoryChanged.AddUniqueDynamic(this, &UEDInventoryComponent::HandleInventoryChangedInternal);
    OnInventoryDropRequested.AddUniqueDynamic(this, &UEDInventoryComponent::HandleDropRequestSpawnWorldItem);

    if (GetOwner() && GetOwner()->HasAuthority() && InventorySlots.Num() != MaxInventorySlots)
    {
        RequestInitializeInventorySlots();
    }

    if (bGiveDefaultWeaponOnBeginPlay)
    {
        RequestEnsureDefaultEquipment();
    }

    if (GetOwner() && GetOwner()->HasAuthority() && bUseEquipmentSlots)
    {
        SyncEquipEffectForSlot(EEDEquippableType::Weapon);
        SyncEquipEffectForSlot(EEDEquippableType::TopArmor);
        SyncEquipEffectForSlot(EEDEquippableType::BottomArmor);
    }

    if (GetOwner() && GetOwner()->HasAuthority() && bAutoInitializeLootOnBeginPlay && !bRandomLootInitialized)
    {
        EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
        RequestInitializeRandomLootDetailed(RandomLootRollCount, RandomLootMinIndex, RandomLootMaxIndex, RandomLootSeed, Failure);
    }

    if (GetOwner() && GetOwner()->HasAuthority() && bAutoDistributeInventoryOnBeginPlay && !bDistributionCompleted)
    {
        TryStartDeferredDistribution();
    }

    RefreshCraftableRecipesCache();

    // ---
    // ?묒꽦??: 源?숈＜
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryDebug: Owner=%s bGiveDebugItemsOnBeginPlay=%s MaxSlots=%d"),
            *GetOwner()->GetName(),
            bGiveDebugItemsOnBeginPlay ? TEXT("true") : TEXT("false"),
            MaxInventorySlots);
    }

    if (GetOwner() && GetOwner()->HasAuthority() && bGiveDebugItemsOnBeginPlay)
    {
        EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;

        UE_LOG(LogTemp, Warning, TEXT("InventoryDebug: Owner=%s ConsumableId=%s MaterialId=%s EquipId=%s"),
            *GetOwner()->GetName(),
            *DebugConsumableItemId.ToString(),
            *DebugMaterialItemId.ToString(),
            *DebugEquipItemId.ToString());

        if (DebugConsumableItemId.IsValid())
        {
            const bool bSuccess = RequestAddItemAutoDetailed(DebugConsumableItemId, 5, Failure);
            UE_LOG(LogTemp, Warning, TEXT("InventoryDebug: Owner=%s AddConsumable success=%s failure=%d"),
                *GetOwner()->GetName(),
                bSuccess ? TEXT("true") : TEXT("false"),
                static_cast<int32>(Failure));
        }

        Failure = EEDInventoryActionFailure::None;

        if (DebugMaterialItemId.IsValid())
        {
            const bool bSuccess = RequestAddItemAutoDetailed(DebugMaterialItemId, 10, Failure);
            UE_LOG(LogTemp, Warning, TEXT("InventoryDebug: Owner=%s AddMaterial success=%s failure=%d"),
                *GetOwner()->GetName(),
                bSuccess ? TEXT("true") : TEXT("false"),
                static_cast<int32>(Failure));
        }

        Failure = EEDInventoryActionFailure::None;

        if (DebugEquipItemId.IsValid())
        {
            const bool bSuccess = RequestAddItemAutoDetailed(DebugEquipItemId, 1, Failure);
            UE_LOG(LogTemp, Warning, TEXT("InventoryDebug: Owner=%s AddEquip success=%s failure=%d"),
                *GetOwner()->GetName(),
                bSuccess ? TEXT("true") : TEXT("false"),
                static_cast<int32>(Failure));
        }
    }
    // ---
}

void UEDInventoryComponent::RequestInitializeInventorySlots()
{
    if (!GetOwner())
    {
        return;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestInitializeInventorySlots();
        return;
    }

    if (MaxInventorySlots < 0)
    {
        MaxInventorySlots = 0;
    }

    InventorySlots.SetNum(MaxInventorySlots);
    OnInventoryChanged.Broadcast();
}

FEDEquipmentSlotData* UEDInventoryComponent::GetEquipmentSlotData(EEDEquippableType SlotType)
{
    switch (SlotType)
    {
    case EEDEquippableType::Weapon:
        return &WeaponSlot;
    case EEDEquippableType::TopArmor:
        return &TopArmorSlot;
    case EEDEquippableType::BottomArmor:
        return &BottomArmorSlot;
    default:
        return nullptr;
    }
}

FActiveGameplayEffectHandle* UEDInventoryComponent::GetEquipmentEffectHandle(EEDEquippableType SlotType)
{
    switch (SlotType)
    {
    case EEDEquippableType::Weapon:
        return &WeaponEquipEffectHandle;
    case EEDEquippableType::TopArmor:
        return &TopArmorEquipEffectHandle;
    case EEDEquippableType::BottomArmor:
        return &BottomArmorEquipEffectHandle;
    default:
        return nullptr;
    }
}

FGameplayTagContainer* UEDInventoryComponent::GetAppliedEquipTagsCache(EEDEquippableType SlotType)
{
    switch (SlotType)
    {
    case EEDEquippableType::Weapon:
        return &WeaponAppliedEquipTags;
    case EEDEquippableType::TopArmor:
        return &TopArmorAppliedEquipTags;
    case EEDEquippableType::BottomArmor:
        return &BottomArmorAppliedEquipTags;
    default:
        return nullptr;
    }
}

bool UEDInventoryComponent::SyncEquipTagsForSlot(EEDEquippableType SlotType)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return false;
    }

    FEDEquipmentSlotData* EquipmentSlot = GetEquipmentSlotData(SlotType);
    FGameplayTagContainer* AppliedTagsCache = GetAppliedEquipTagsCache(SlotType);
    if (!EquipmentSlot || !AppliedTagsCache)
    {
        return false;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
    if (!ASC)
    {
        AppliedTagsCache->Reset();
        return false;
    }

    FGameplayTagContainer DesiredTags;
    if (EquipmentSlot->EquippedItem.IsValid())
    {
        const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(EquipmentSlot->EquippedItem.ItemId);
        if (ItemData)
        {
            DesiredTags.AppendTags(ItemData->ItemTags);
            DesiredTags.AppendTags(ItemData->ItemSpecialTags);
        }
    }

    RemoveLooseTags(ASC, *AppliedTagsCache);
    AddLooseTags(ASC, DesiredTags);
    *AppliedTagsCache = DesiredTags;
    return true;
}

bool UEDInventoryComponent::SyncEquipEffectForSlot(EEDEquippableType SlotType)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return false;
    }

    FEDEquipmentSlotData* EquipmentSlot = GetEquipmentSlotData(SlotType);
    FActiveGameplayEffectHandle* EffectHandle = GetEquipmentEffectHandle(SlotType);
    if (!EquipmentSlot || !EffectHandle)
    {
        return false;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
    if (!ASC)
    {
        EffectHandle->Invalidate();
        FGameplayTagContainer* AppliedTagsCache = GetAppliedEquipTagsCache(SlotType);
        if (AppliedTagsCache)
        {
            AppliedTagsCache->Reset();
        }
        return false;
    }

    if (EffectHandle->IsValid())
    {
        ASC->RemoveActiveGameplayEffect(*EffectHandle);
        EffectHandle->Invalidate();
    }

    if (!EquipmentSlot->EquippedItem.IsValid())
    {
        return SyncEquipTagsForSlot(SlotType);
    }

    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(EquipmentSlot->EquippedItem.ItemId);
    if (!ItemData || !ItemData->EquipEffectClass)
    {
        return SyncEquipTagsForSlot(SlotType);
    }

    *EffectHandle = UEDInventoryGASBridge::ApplyEquipEffectWithHandle(GetOwner(), ASC, ItemData);
    const bool bEffectApplied = EffectHandle->WasSuccessfullyApplied();
    if (!bEffectApplied)
    {
        EffectHandle->Invalidate();
    }

    const bool bTagsSynced = SyncEquipTagsForSlot(SlotType);
    return bEffectApplied && bTagsSynced;
}

bool UEDInventoryComponent::PredicateMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex) || !InventorySlots.IsValidIndex(ToSlotIndex))
    {
        OutFailure = EEDInventoryActionFailure::InvalidSlot;
        return false;
    }

    if (FromSlotIndex == ToSlotIndex)
    {
        OutFailure = EEDInventoryActionFailure::SlotConflict;
        return false;
    }

    if (InventorySlots[FromSlotIndex].IsEmpty())
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    return bAutoRequestIfValid ? RequestMoveItemBetweenSlots(FromSlotIndex, ToSlotIndex) : true;
}

bool UEDInventoryComponent::PredicateTransferItemAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!PrecheckTransferAuto_Component(FromInventory, ToInventory, FromSlotIndex, Quantity, &OutFailure))
    {
        return false;
    }

    return bAutoRequestIfValid ? RequestTransferItemAuto(FromInventory, ToInventory, FromSlotIndex, Quantity) : true;
}

bool UEDInventoryComponent::PredicateTransferItemToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!PrecheckTransferToSlot_Component(FromInventory, ToInventory, FromSlotIndex, ToSlotIndex, Quantity, &OutFailure))
    {
        return false;
    }

    return bAutoRequestIfValid ? RequestTransferItemToSlot(FromInventory, ToInventory, FromSlotIndex, ToSlotIndex, Quantity) : true;
}

bool UEDInventoryComponent::PredicateDropAllFromSlot(int32 FromSlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex))
    {
        OutFailure = EEDInventoryActionFailure::InvalidSlot;
        return false;
    }

    if (InventorySlots[FromSlotIndex].IsEmpty())
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    return bAutoRequestIfValid ? RequestDropAllFromSlot(FromSlotIndex) : true;
}

bool UEDInventoryComponent::PredicateDropSingleFromSlot(int32 FromSlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    const bool bValid = PredicateDropAllFromSlot(FromSlotIndex, OutFailure, false);
    if (!bValid)
    {
        return false;
    }

    return bAutoRequestIfValid ? RequestDropSingleFromSlot(FromSlotIndex) : true;
}

bool UEDInventoryComponent::PredicateDropCountFromSlot(int32 FromSlotIndex, int32 DropCount, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (DropCount <= 0)
    {
        OutFailure = EEDInventoryActionFailure::InvalidQuantity;
        return false;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex))
    {
        OutFailure = EEDInventoryActionFailure::InvalidSlot;
        return false;
    }

    const FEDInventorySlotData& Slot = InventorySlots[FromSlotIndex];
    if (Slot.IsEmpty())
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    return bAutoRequestIfValid ? RequestDropCountFromSlot(FromSlotIndex, DropCount) : true;
}

bool UEDInventoryComponent::PredicatePickupDroppedItem(AEDDroppedItemActor* DroppedItemActor, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!IsValid(DroppedItemActor))
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    const FPrimaryAssetId ItemId = DroppedItemActor->GetItemId();
    const int32 Quantity = DroppedItemActor->GetQuantity();
    if (!ItemId.IsValid() || Quantity <= 0)
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (GetReceivableCapacity_Component(this, ItemId) < Quantity)
    {
        OutFailure = EEDInventoryActionFailure::NoSpace;
        return false;
    }

    return bAutoRequestIfValid ? RequestPickupDroppedItem(DroppedItemActor) : true;
}

bool UEDInventoryComponent::PredicateEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex))
    {
        OutFailure = EEDInventoryActionFailure::InvalidSlot;
        return false;
    }

    const FEDInventorySlotData& SourceSlot = InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(SourceSlot.Item.ItemId);
    if (!ItemData)
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (!FEDInventoryValidationService::CanEquipToSlot(ItemData, TargetSlotType))
    {
        OutFailure = EEDInventoryActionFailure::SlotConflict;
        return false;
    }

    return bAutoRequestIfValid ? RequestEquipItemFromSlot(FromSlotIndex, TargetSlotType) : true;
}

bool UEDInventoryComponent::PredicateUnequipTopArmor(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (TopArmorSlot.EquippedItem.IsValid() == false)
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    return bAutoRequestIfValid ? RequestUnequipTopArmor() : true;
}

bool UEDInventoryComponent::PredicateUnequipBottomArmor(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (BottomArmorSlot.EquippedItem.IsValid() == false)
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    return bAutoRequestIfValid ? RequestUnequipBottomArmor() : true;
}

bool UEDInventoryComponent::PredicateAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!ItemId.IsValid() || !ResolveItemData_Component(ItemId))
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (Quantity <= 0)
    {
        OutFailure = EEDInventoryActionFailure::InvalidQuantity;
        return false;
    }

    if (GetReceivableCapacity_Component(this, ItemId) < Quantity)
    {
        OutFailure = EEDInventoryActionFailure::NoSpace;
        return false;
    }

    return bAutoRequestIfValid ? RequestAddItemAuto(ItemId, Quantity) : true;
}

bool UEDInventoryComponent::PredicateAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!ItemId.IsValid() || !ResolveItemData_Component(ItemId))
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (Quantity <= 0)
    {
        OutFailure = EEDInventoryActionFailure::InvalidQuantity;
        return false;
    }

    if (!InventorySlots.IsValidIndex(SlotIndex))
    {
        OutFailure = EEDInventoryActionFailure::InvalidSlot;
        return false;
    }

    const FEDInventorySlotData& Slot = InventorySlots[SlotIndex];
    const int32 MaxStack = GetItemMaxStack_Component(ItemId);
    if (Slot.IsEmpty())
    {
        if (Quantity > MaxStack)
        {
            OutFailure = EEDInventoryActionFailure::StackLimit;
            return false;
        }
    }
    else
    {
        if (Slot.Item.ItemId != ItemId)
        {
            OutFailure = EEDInventoryActionFailure::SlotConflict;
            return false;
        }

        if (Quantity > FMath::Max(0, MaxStack - Slot.Item.Quantity))
        {
            OutFailure = EEDInventoryActionFailure::StackLimit;
            return false;
        }
    }

    return bAutoRequestIfValid ? RequestAddItemToSlot(ItemId, Quantity, SlotIndex) : true;
}

bool UEDInventoryComponent::PredicateInitializeRandomLoot(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!RandomLootTable)
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (RandomLootSpawnMode == EEDInventoryLootSpawnMode::QuantityMax && RandomLootMaxTotalQuantity <= 0)
    {
        OutFailure = EEDInventoryActionFailure::InvalidQuantity;
        return false;
    }

    if (RandomLootSpawnMode == EEDInventoryLootSpawnMode::RollCountMax && RandomLootRollCount <= 0)
    {
        OutFailure = EEDInventoryActionFailure::InvalidQuantity;
        return false;
    }

    return bAutoRequestIfValid ? RequestInitializeRandomLoot() : true;
}

bool UEDInventoryComponent::PredicateDistributeInventoryToTargets(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!IsReadyForDistribution())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    TArray<UEDInventoryComponent*> ReadyTargets;
    bool bHasPendingTargets = false;
    GatherDistributionTargets(ReadyTargets, bHasPendingTargets);
    if (bHasPendingTargets)
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    return bAutoRequestIfValid ? RequestDistributeInventoryToTargets() : true;
}

bool UEDInventoryComponent::PredicateCraftItem(FName RecipeId, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    FEDCraftingRecipeRow RecipeRow;
    if (!TryFindRecipeById_Component(this, RecipeId, RecipeRow))
    {
        OutFailure = EEDInventoryActionFailure::InvalidRecipe;
        return false;
    }

    if (!FEDInventoryCraftService::CanCraftRecipe(this, RecipeRow, &OutFailure))
    {
        return false;
    }

    return bAutoRequestIfValid ? RequestCraftItem(RecipeId) : true;
}

bool UEDInventoryComponent::PredicateConsumeItemAtSlot(int32 SlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!InventorySlots.IsValidIndex(SlotIndex))
    {
        OutFailure = EEDInventoryActionFailure::InvalidSlot;
        return false;
    }

    if (InventorySlots[SlotIndex].IsEmpty())
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(InventorySlots[SlotIndex].Item.ItemId);
    if (!FEDInventoryValidationService::CanConsumeItem(ItemData, GetOwner(), &OutFailure))
    {
        return false;
    }

    return bAutoRequestIfValid ? RequestConsumeItemAtSlot(SlotIndex) : true;
}

bool UEDInventoryComponent::PredicateEnsureDefaultEquipment(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    return bAutoRequestIfValid ? RequestEnsureDefaultEquipment() : true;
}

bool UEDInventoryComponent::RequestMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestMoveItemBetweenSlots(FromSlotIndex, ToSlotIndex);
        return true;
    }

    const bool bSucceeded = FEDInventoryTransferService::MoveOrSwap(this, FromSlotIndex, ToSlotIndex);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestTransferItemAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return RequestTransferItemAutoDetailed(FromInventory, ToInventory, FromSlotIndex, Quantity, Failure);
}

bool UEDInventoryComponent::RequestTransferItemAutoDetailed(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!FromInventory || !ToInventory)
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!PrecheckTransferAuto_Component(FromInventory, ToInventory, FromSlotIndex, Quantity, &OutFailure))
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestTransferItemAuto(FromInventory, ToInventory, FromSlotIndex, Quantity);
        return true;
    }

    const bool bSucceeded = FEDInventoryTransferService::TransferAuto(FromInventory, ToInventory, FromSlotIndex, Quantity, &OutFailure);
    if (bSucceeded)
    {
        FromInventory->OnInventoryChanged.Broadcast();
        if (ToInventory != FromInventory)
        {
            ToInventory->OnInventoryChanged.Broadcast();
        }
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestTransferItemToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return RequestTransferItemToSlotDetailed(FromInventory, ToInventory, FromSlotIndex, ToSlotIndex, Quantity, Failure);
}

bool UEDInventoryComponent::RequestTransferItemToSlotDetailed(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!FromInventory || !ToInventory)
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!PrecheckTransferToSlot_Component(FromInventory, ToInventory, FromSlotIndex, ToSlotIndex, Quantity, &OutFailure))
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestTransferItemToSlot(FromInventory, ToInventory, FromSlotIndex, ToSlotIndex, Quantity);
        return true;
    }

    const bool bSucceeded = FEDInventoryTransferService::TransferToSlot(FromInventory, ToInventory, FromSlotIndex, ToSlotIndex, Quantity, &OutFailure);
    if (bSucceeded)
    {
        FromInventory->OnInventoryChanged.Broadcast();
        if (ToInventory != FromInventory)
        {
            ToInventory->OnInventoryChanged.Broadcast();
        }
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestDropAllFromSlot(int32 FromSlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestDropAllFromSlot(FromSlotIndex);
        return true;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex))
    {
        return false;
    }

    if (InventorySlots[FromSlotIndex].IsEmpty())
    {
        return false;
    }

    FEDInventoryDropRequest DropRequest;
    DropRequest.Item = InventorySlots[FromSlotIndex].Item;
    DropRequest.SourceOwner = GetOwner();
    DropRequest.Reason = EEDInventoryDropReason::UserRequested;

    InventorySlots[FromSlotIndex].Item = FEDInventoryItemHandle();
    OnInventoryDropRequested.Broadcast(DropRequest);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UEDInventoryComponent::RequestDropSingleFromSlot(int32 FromSlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestDropSingleFromSlot(FromSlotIndex);
        return true;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex))
    {
        return false;
    }

    if (InventorySlots[FromSlotIndex].IsEmpty())
    {
        return false;
    }

    FEDInventoryDropRequest DropRequest;
    DropRequest.Item.ItemId = InventorySlots[FromSlotIndex].Item.ItemId;
    DropRequest.Item.Quantity = 1;
    DropRequest.SourceOwner = GetOwner();
    DropRequest.Reason = EEDInventoryDropReason::UserRequested;

    InventorySlots[FromSlotIndex].Item.Quantity -= 1;
    if (InventorySlots[FromSlotIndex].Item.Quantity <= 0)
    {
        InventorySlots[FromSlotIndex].Item = FEDInventoryItemHandle();
    }

    OnInventoryDropRequested.Broadcast(DropRequest);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UEDInventoryComponent::RequestDropCountFromSlot(int32 FromSlotIndex, int32 DropCount)
{
    if (!GetOwner())
    {
        return false;
    }

    if (DropCount <= 0)
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestDropCountFromSlot(FromSlotIndex, DropCount);
        return true;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex))
    {
        return false;
    }

    if (InventorySlots[FromSlotIndex].IsEmpty())
    {
        return false;
    }

    FEDInventoryItemHandle& SlotItem = InventorySlots[FromSlotIndex].Item;
    const int32 ActualDropCount = FMath::Min(DropCount, SlotItem.Quantity);
    if (ActualDropCount <= 0)
    {
        return false;
    }

    FEDInventoryDropRequest DropRequest;
    DropRequest.Item.ItemId = SlotItem.ItemId;
    DropRequest.Item.Quantity = ActualDropCount;
    DropRequest.SourceOwner = GetOwner();
    DropRequest.Reason = EEDInventoryDropReason::UserRequested;

    SlotItem.Quantity -= ActualDropCount;
    if (SlotItem.Quantity <= 0)
    {
        SlotItem = FEDInventoryItemHandle();
    }

    OnInventoryDropRequested.Broadcast(DropRequest);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UEDInventoryComponent::RequestPickupDroppedItem(AEDDroppedItemActor* DroppedItemActor)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return RequestPickupDroppedItemDetailed(DroppedItemActor, Failure);
}

bool UEDInventoryComponent::RequestPickupDroppedItemDetailed(AEDDroppedItemActor* DroppedItemActor, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!IsValid(DroppedItemActor))
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestPickupDroppedItem(DroppedItemActor);
        return true;
    }

    const FPrimaryAssetId ItemId = DroppedItemActor->GetItemId();
    const int32 Quantity = DroppedItemActor->GetQuantity();
    if (!ItemId.IsValid() || Quantity <= 0)
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (!RequestAddItemAutoDetailed(ItemId, Quantity, OutFailure))
    {
        return false;
    }

    if (IsValid(DroppedItemActor))
    {
        DroppedItemActor->Destroy();
    }

    return true;
}

bool UEDInventoryComponent::RequestEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex))
    {
        return false;
    }

    const FEDInventorySlotData& SourceSlot = InventorySlots[FromSlotIndex];
    if (SourceSlot.IsEmpty())
    {
        return false;
    }

    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(SourceSlot.Item.ItemId);
    if (!FEDInventoryValidationService::CanEquipToSlot(ItemData, TargetSlotType))
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestEquipItemFromSlot(FromSlotIndex, TargetSlotType);
        return true;
    }

    const bool bSucceeded = FEDInventoryEquipmentService::EquipFromSlot(this, FromSlotIndex, TargetSlotType);
    if (bSucceeded)
    {
        SyncEquipEffectForSlot(TargetSlotType);
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestUnequipTopArmor()
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestUnequipTopArmor();
        return true;
    }

    const bool bSucceeded = FEDInventoryEquipmentService::UnequipTopArmor(this);
    if (bSucceeded)
    {
        SyncEquipEffectForSlot(EEDEquippableType::TopArmor);
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestUnequipBottomArmor()
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestUnequipBottomArmor();
        return true;
    }

    const bool bSucceeded = FEDInventoryEquipmentService::UnequipBottomArmor(this);
    if (bSucceeded)
    {
        SyncEquipEffectForSlot(EEDEquippableType::BottomArmor);
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestCraftItem(FName RecipeId)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return RequestCraftItemDetailed(RecipeId, Failure);
}

bool UEDInventoryComponent::RequestCraftItemDetailed(FName RecipeId, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    FEDCraftingRecipeRow RecipeRow;
    if (!TryFindRecipeById_Component(this, RecipeId, RecipeRow))
    {
        OutFailure = EEDInventoryActionFailure::InvalidRecipe;
        return false;
    }

    if (!FEDInventoryCraftService::CanCraftRecipe(this, RecipeRow, &OutFailure))
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestCraftItem(RecipeId);
        return true;
    }

    const bool bSucceeded = FEDInventoryCraftService::TryCraftByRecipeId(this, RecipeId, &OutFailure);
    if (bSucceeded)
    {
        if (bUseEquipmentSlots)
        {
            SyncEquipEffectForSlot(EEDEquippableType::Weapon);
            SyncEquipEffectForSlot(EEDEquippableType::TopArmor);
            SyncEquipEffectForSlot(EEDEquippableType::BottomArmor);
        }
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

void UEDInventoryComponent::GetCraftableRecipes(TArray<FEDCraftableRecipeEntry>& OutRecipes, EEDCraftableRecipeSortOption SortOption, bool bDescending) const
{
    FEDInventoryCraftService::GetCraftableRecipes(this, OutRecipes, SortOption, bDescending);
}

void UEDInventoryComponent::GetAllCraftingRecipeTables(TArray<UDataTable*>& OutTables) const
{
    OutTables.Reset();

    for (UDataTable* RecipeTable : CraftingRecipeTables)
    {
        if (RecipeTable)
        {
            OutTables.AddUnique(RecipeTable);
        }
    }
}

bool UEDInventoryComponent::CanCraftRecipeByRowId(FName RecipeRowId) const
{
    if (RecipeRowId.IsNone())
    {
        return false;
    }

    TArray<UDataTable*> RecipeTables;
    GetAllCraftingRecipeTables(RecipeTables);

    for (UDataTable* RecipeTable : RecipeTables)
    {
        if (!RecipeTable)
        {
            continue;
        }

        const FEDCraftingRecipeRow* RecipeRow = RecipeTable->FindRow<FEDCraftingRecipeRow>(RecipeRowId, TEXT("CanCraftRecipeByRowId"));
        if (RecipeRow)
        {
            return FEDInventoryCraftService::CanCraftRecipe(this, *RecipeRow, nullptr);
        }
    }

    return false;
}

void UEDInventoryComponent::RefreshCraftableRecipesCache()
{
    FEDInventoryCraftService::GetCraftableRecipes(this, CachedCraftableRecipes, CachedCraftableRecipesSortOption, bCachedCraftableRecipesDescending);
}

void UEDInventoryComponent::GetCachedCraftableRecipes(TArray<FEDCraftableRecipeEntry>& OutRecipes) const
{
    OutRecipes = CachedCraftableRecipes;
}

void UEDInventoryComponent::SetCraftableRecipeCacheSort(EEDCraftableRecipeSortOption SortOption, bool bDescending)
{
    CachedCraftableRecipesSortOption = SortOption;
    bCachedCraftableRecipesDescending = bDescending;
    RefreshCraftableRecipesCache();
}

bool UEDInventoryComponent::RequestCraftFirstCachedRecipe()
{
    if (CachedCraftableRecipes.Num() <= 0)
    {
        return false;
    }

    const FName RecipeRowId = CachedCraftableRecipes[0].RowId;
    if (RecipeRowId.IsNone())
    {
        return false;
    }

    return RequestCraftItem(RecipeRowId);
}

bool UEDInventoryComponent::RequestConsumeItemAtSlot(int32 SlotIndex)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return RequestConsumeItemAtSlotDetailed(SlotIndex, Failure);
}

bool UEDInventoryComponent::RequestConsumeItemAtSlotDetailed(int32 SlotIndex, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!InventorySlots.IsValidIndex(SlotIndex))
    {
        OutFailure = EEDInventoryActionFailure::InvalidSlot;
        return false;
    }

    if (InventorySlots[SlotIndex].IsEmpty())
    {
        OutFailure = EEDInventoryActionFailure::EmptySlot;
        return false;
    }

    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(InventorySlots[SlotIndex].Item.ItemId);
    if (!FEDInventoryValidationService::CanConsumeItem(ItemData, GetOwner(), &OutFailure))
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestConsumeItemAtSlot(SlotIndex);
        return true;
    }

    if (ItemData && ItemData->ConsumableEffectClass)
    {
        UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
        if (!UEDInventoryGASBridge::ApplyConsumableEffect(GetOwner(), ASC, ItemData))
        {
            OutFailure = EEDInventoryActionFailure::EffectApplyFailed;
            return false;
        }
    }

    InventorySlots[SlotIndex].Item.Quantity -= 1;
    if (InventorySlots[SlotIndex].Item.Quantity <= 0)
    {
        InventorySlots[SlotIndex].Item = FEDInventoryItemHandle();
    }

    OnInventoryChanged.Broadcast();
    return true;
}

bool UEDInventoryComponent::RequestEnsureDefaultEquipment()
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestEnsureDefaultEquipment();
        return true;
    }

    const bool bSucceeded = FEDInventoryEquipmentService::EnsureDefaultWeapon(this);
    if (bSucceeded)
    {
        SyncEquipEffectForSlot(EEDEquippableType::Weapon);
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity)
{
    if (!GetOwner())
    {
        return false;
    }

    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    if (!PredicateAddItemAuto(ItemId, Quantity, Failure, false))
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestAddItemAuto(ItemId, Quantity);
        return true;
    }

    const bool bSucceeded = AddItemAuto_Component(this, ItemId, Quantity);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestAddItemAutoDetailed(FPrimaryAssetId ItemId, int32 Quantity, EEDInventoryActionFailure& OutFailure)
{
    if (!PredicateAddItemAuto(ItemId, Quantity, OutFailure, false))
    {
        return false;
    }

    return RequestAddItemAuto(ItemId, Quantity);
}

bool UEDInventoryComponent::RequestAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    if (!PredicateAddItemToSlot(ItemId, Quantity, SlotIndex, Failure, false))
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestAddItemToSlot(ItemId, Quantity, SlotIndex);
        return true;
    }

    const bool bSucceeded = AddItemToSlot_Component(this, ItemId, Quantity, SlotIndex, &Failure);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::RequestAddItemToSlotDetailed(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure& OutFailure)
{
    if (!PredicateAddItemToSlot(ItemId, Quantity, SlotIndex, OutFailure, false))
    {
        return false;
    }

    return RequestAddItemToSlot(ItemId, Quantity, SlotIndex);
}

bool UEDInventoryComponent::RequestInitializeRandomLoot()
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return RequestInitializeRandomLootDetailed(RandomLootRollCount, RandomLootMinIndex, RandomLootMaxIndex, RandomLootSeed, Failure);
}

bool UEDInventoryComponent::RequestInitializeRandomLootDetailed(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!RandomLootTable)
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestInitializeRandomLoot(RollCount, MinLootIndex, MaxLootIndex, Seed);
        return true;
    }

    if (MaxLootIndex < MinLootIndex)
    {
        Swap(MinLootIndex, MaxLootIndex);
    }

    TArray<FEDItemSpawnRow*> AllRows;
    RandomLootTable->GetAllRows<FEDItemSpawnRow>(TEXT("InventoryRandomLootInit"), AllRows);

    TArray<const FEDItemSpawnRow*> CandidateRows;
    CandidateRows.Reserve(AllRows.Num());

    for (const FEDItemSpawnRow* Row : AllRows)
    {
        if (!Row || !Row->ItemId.IsValid())
        {
            continue;
        }

        if (Row->LootIndex < MinLootIndex || Row->LootIndex > MaxLootIndex)
        {
            continue;
        }

        if (Row->Weight <= 0.0f)
        {
            continue;
        }

        CandidateRows.Add(Row);
    }

    if (CandidateRows.Num() == 0)
    {
        OutFailure = EEDInventoryActionFailure::MissingData;
        return false;
    }

    FRandomStream RandomStream = BuildRandomStreamFromOptionalSeed(Seed);

    switch (RandomLootSpawnMode)
    {
    case EEDInventoryLootSpawnMode::Static:
        {
            TArray<const FEDItemSpawnRow*> OrderedRows = CandidateRows;
            OrderedRows.Sort([](const FEDItemSpawnRow& A, const FEDItemSpawnRow& B)
            {
                if (!FMath::IsNearlyEqual(A.Weight, B.Weight))
                {
                    return A.Weight > B.Weight;
                }

                if (A.LootIndex != B.LootIndex)
                {
                    return A.LootIndex < B.LootIndex;
                }

                return FCString::Stricmp(*A.ItemId.ToString(), *B.ItemId.ToString()) < 0;
            });

            for (const FEDItemSpawnRow* Row : OrderedRows)
            {
                if (!Row)
                {
                    continue;
                }

                const int32 SpawnMin = FMath::Min(Row->MinCount, Row->MaxCount);
                const int32 SpawnMax = FMath::Max(Row->MinCount, Row->MaxCount);
                const int32 SpawnCount = FMath::Max(1, SpawnMax > 0 ? SpawnMax : SpawnMin);

                if (!AddItemAuto_Component(this, Row->ItemId, SpawnCount))
                {
                    OutFailure = EEDInventoryActionFailure::NoSpace;
                    return false;
                }
            }
        }
        break;

    case EEDInventoryLootSpawnMode::QuantityMax:
        {
            if (RandomLootMaxTotalQuantity <= 0)
            {
                OutFailure = EEDInventoryActionFailure::InvalidQuantity;
                return false;
            }

            int32 RemainingTotalQuantity = RandomLootMaxTotalQuantity;
            int32 GuardCount = 0;
            const int32 GuardLimit = FMath::Max(32, RandomLootMaxTotalQuantity * 8);

            while (RemainingTotalQuantity > 0 && GuardCount < GuardLimit)
            {
                ++GuardCount;

                const FEDItemSpawnRow* SelectedRow = PickLootRowByWeight_Component(CandidateRows, RandomStream);
                if (!SelectedRow)
                {
                    OutFailure = EEDInventoryActionFailure::MissingData;
                    return false;
                }

                const int32 SpawnMin = FMath::Min(SelectedRow->MinCount, SelectedRow->MaxCount);
                const int32 SpawnMax = FMath::Max(SelectedRow->MinCount, SelectedRow->MaxCount);
                int32 SpawnCount = RandomStream.RandRange(SpawnMin, SpawnMax);
                SpawnCount = FMath::Clamp(SpawnCount, 1, RemainingTotalQuantity);

                if (!AddItemAuto_Component(this, SelectedRow->ItemId, SpawnCount))
                {
                    OutFailure = EEDInventoryActionFailure::NoSpace;
                    return false;
                }

                RemainingTotalQuantity -= SpawnCount;
            }
        }
        break;

    case EEDInventoryLootSpawnMode::RollCountMax:
    default:
        {
            if (RollCount <= 0)
            {
                OutFailure = EEDInventoryActionFailure::InvalidQuantity;
                return false;
            }

            for (int32 Roll = 0; Roll < RollCount; ++Roll)
            {
                const FEDItemSpawnRow* SelectedRow = PickLootRowByWeight_Component(CandidateRows, RandomStream);
                if (!SelectedRow)
                {
                    OutFailure = EEDInventoryActionFailure::MissingData;
                    return false;
                }

                const int32 SpawnMin = FMath::Min(SelectedRow->MinCount, SelectedRow->MaxCount);
                const int32 SpawnMax = FMath::Max(SelectedRow->MinCount, SelectedRow->MaxCount);
                const int32 SpawnCount = RandomStream.RandRange(SpawnMin, SpawnMax);

                if (!AddItemAuto_Component(this, SelectedRow->ItemId, SpawnCount))
                {
                    OutFailure = EEDInventoryActionFailure::NoSpace;
                    return false;
                }
            }
        }
        break;
    }

    bRandomLootInitialized = true;
    OnInventoryChanged.Broadcast();
    return true;
}

bool UEDInventoryComponent::RequestDistributeInventoryToTargets()
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return RequestDistributeInventoryToTargetsDetailed(Failure);
}

bool UEDInventoryComponent::RequestDistributeInventoryToTargetsDetailed(EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestDistributeInventoryToTargets();
        return true;
    }

    TArray<UEDInventoryComponent*> ReadyTargets;
    bool bHasPendingTargets = false;
    GatherDistributionTargets(ReadyTargets, bHasPendingTargets);

    if (!IsReadyForDistribution() || bHasPendingTargets)
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    const bool bSucceeded = ExecuteDistribution(ReadyTargets, OutFailure);
    if (bSucceeded)
    {
        bDistributionCompleted = true;
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::IsInventoryReadyForDistribution(const UEDInventoryComponent* InventoryComponent)
{
    if (!InventoryComponent)
    {
        return false;
    }

    if (InventoryComponent->InventorySlots.Num() != InventoryComponent->MaxInventorySlots)
    {
        return false;
    }

    if (InventoryComponent->bAutoInitializeLootOnBeginPlay && InventoryComponent->RandomLootTable && !InventoryComponent->bRandomLootInitialized)
    {
        return false;
    }

    return true;
}

bool UEDInventoryComponent::IsReadyForDistribution() const
{
    return IsInventoryReadyForDistribution(this);
}

void UEDInventoryComponent::GatherDistributionTargets(TArray<UEDInventoryComponent*>& OutReadyTargets, bool& bOutHasPendingTargets) const
{
    OutReadyTargets.Reset();
    bOutHasPendingTargets = false;

    if (!GetOwner())
    {
        return;
    }

    TSet<UEDInventoryComponent*> UniqueTargets;

    for (AActor* TargetActor : DistributionTargetActors)
    {
        UEDInventoryComponent* TargetInventory = ResolveInventoryComponentFromActor_Component(TargetActor);
        if (!TargetInventory || TargetInventory == this)
        {
            continue;
        }

        if (UniqueTargets.Contains(TargetInventory))
        {
            continue;
        }
        UniqueTargets.Add(TargetInventory);

        if (IsInventoryReadyForDistribution(TargetInventory))
        {
            OutReadyTargets.Add(TargetInventory);
        }
        else
        {
            bOutHasPendingTargets = true;
        }
    }
}

bool UEDInventoryComponent::ExecuteDistribution(const TArray<UEDInventoryComponent*>& ReadyTargets, EEDInventoryActionFailure& OutFailure)
{
    if (ReadyTargets.Num() == 0)
    {
        return true;
    }

    FRandomStream RandomStream = BuildRandomStreamFromOptionalSeed(DistributionSeed);

    switch (DistributionMode)
    {
    case EEDInventorySplitMode::ByType:
        return ExecuteDistributionByType(ReadyTargets, RandomStream);
    case EEDInventorySplitMode::ByCount:
    default:
        return ExecuteDistributionByCount(ReadyTargets, RandomStream);
    }
}

bool UEDInventoryComponent::ExecuteDistributionByCount(const TArray<UEDInventoryComponent*>& ReadyTargets, FRandomStream& RandomStream)
{
    bool bMovedAny = false;
    const bool bUseRaritySorting = IsRaritySortingEnabled_Component(this);

    for (int32 SlotIndex = 0; SlotIndex < InventorySlots.Num(); ++SlotIndex)
    {
        while (InventorySlots.IsValidIndex(SlotIndex) && !InventorySlots[SlotIndex].IsEmpty())
        {
            const FPrimaryAssetId ItemId = InventorySlots[SlotIndex].Item.ItemId;
            if (!ItemId.IsValid())
            {
                break;
            }

            TArray<UEDInventoryComponent*> Candidates;
            for (UEDInventoryComponent* Target : ReadyTargets)
            {
                if (GetReceivableCapacity_Component(Target, ItemId) > 0)
                {
                    Candidates.Add(Target);
                }
            }

            if (Candidates.Num() == 0)
            {
                break;
            }

            UEDInventoryComponent* PickedTarget = nullptr;
            if (bUseRaritySorting)
            {
                PickedTarget = PickCandidateWithLowestRarityLoad_Component(
                    Candidates,
                    ResolveItemRarity_Component(ItemId),
                    EEDInventorySplitMode::ByCount,
                    RandomStream);
            }
            else
            {
                PickedTarget = Candidates[RandomStream.RandRange(0, Candidates.Num() - 1)];
            }

            if (!PickedTarget)
            {
                break;
            }

            const int32 MoveQuantity = ResolveDistributionMoveQuantity_Component(
                InventorySlots[SlotIndex].Item.Quantity,
                GetReceivableCapacity_Component(PickedTarget, ItemId),
                DistributionStackPolicy,
                RandomStream);
            if (MoveQuantity <= 0)
            {
                break;
            }

            EEDInventoryActionFailure TransferFailure = EEDInventoryActionFailure::None;
            if (!FEDInventoryTransferService::TransferAuto(this, PickedTarget, SlotIndex, MoveQuantity, &TransferFailure, DistributionStackPolicy, &RandomStream))
            {
                break;
            }

            bMovedAny = true;
        }
    }

    if (bMovedAny)
    {
        for (UEDInventoryComponent* Target : ReadyTargets)
        {
            if (Target)
            {
                Target->OnInventoryChanged.Broadcast();
            }
        }
        OnInventoryChanged.Broadcast();
    }

    return true;
}

bool UEDInventoryComponent::ExecuteDistributionByType(const TArray<UEDInventoryComponent*>& ReadyTargets, FRandomStream& RandomStream)
{
    bool bMovedAny = false;
    const bool bUseRaritySorting = IsRaritySortingEnabled_Component(this);

    TSet<FPrimaryAssetId> UniqueItemIds;
    for (const FEDInventorySlotData& Slot : InventorySlots)
    {
        if (!Slot.IsEmpty() && Slot.Item.ItemId.IsValid())
        {
            UniqueItemIds.Add(Slot.Item.ItemId);
        }
    }

    for (const FPrimaryAssetId& ItemId : UniqueItemIds)
    {
        TArray<UEDInventoryComponent*> OrderedTargets;
        for (UEDInventoryComponent* Target : ReadyTargets)
        {
            if (GetReceivableCapacity_Component(Target, ItemId) > 0)
            {
                OrderedTargets.Add(Target);
            }
        }

        if (OrderedTargets.Num() == 0)
        {
            continue;
        }

        if (bUseRaritySorting)
        {
            TArray<UEDInventoryComponent*> RemainingTargets = OrderedTargets;
            OrderedTargets.Reset();
            const EEDItemRarity ItemRarity = ResolveItemRarity_Component(ItemId);

            while (RemainingTargets.Num() > 0)
            {
                UEDInventoryComponent* Picked = PickCandidateWithLowestRarityLoad_Component(
                    RemainingTargets,
                    ItemRarity,
                    EEDInventorySplitMode::ByType,
                    RandomStream);
                if (!Picked)
                {
                    break;
                }

                OrderedTargets.Add(Picked);
                RemainingTargets.RemoveSingleSwap(Picked);
            }
        }
        else
        {
            ShuffleArray_Component(OrderedTargets, RandomStream);
        }

        for (int32 SlotIndex = 0; SlotIndex < InventorySlots.Num(); ++SlotIndex)
        {
            while (InventorySlots.IsValidIndex(SlotIndex) && !InventorySlots[SlotIndex].IsEmpty() && InventorySlots[SlotIndex].Item.ItemId == ItemId)
            {
                bool bTransferredThisStep = false;
                for (UEDInventoryComponent* Target : OrderedTargets)
                {
                    if (!Target)
                    {
                        continue;
                    }

                    const int32 Capacity = GetReceivableCapacity_Component(Target, ItemId);
                    if (Capacity <= 0)
                    {
                        continue;
                    }

                    const int32 MoveQuantity = ResolveDistributionMoveQuantity_Component(
                        InventorySlots[SlotIndex].Item.Quantity,
                        Capacity,
                        DistributionStackPolicy,
                        RandomStream);
                    if (MoveQuantity <= 0)
                    {
                        continue;
                    }

                    EEDInventoryActionFailure TransferFailure = EEDInventoryActionFailure::None;
                    if (!FEDInventoryTransferService::TransferAuto(this, Target, SlotIndex, MoveQuantity, &TransferFailure, DistributionStackPolicy, &RandomStream))
                    {
                        continue;
                    }

                    bMovedAny = true;
                    bTransferredThisStep = true;
                    if (InventorySlots[SlotIndex].IsEmpty())
                    {
                        break;
                    }
                }

                if (!bTransferredThisStep)
                {
                    break;
                }
            }
        }
    }

    if (bMovedAny)
    {
        for (UEDInventoryComponent* Target : ReadyTargets)
        {
            if (Target)
            {
                Target->OnInventoryChanged.Broadcast();
            }
        }
        OnInventoryChanged.Broadcast();
    }

    return true;
}

void UEDInventoryComponent::TryStartDeferredDistribution()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || bDistributionCompleted || !GetWorld())
    {
        return;
    }

    DeferredDistributionStartTime = GetWorld()->GetTimeSeconds();

    TArray<UEDInventoryComponent*> ReadyTargets;
    bool bHasPendingTargets = false;
    GatherDistributionTargets(ReadyTargets, bHasPendingTargets);

    const bool bSourceReady = IsReadyForDistribution();
    if (bSourceReady && !bHasPendingTargets)
    {
        EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
        bDistributionCompleted = ExecuteDistribution(ReadyTargets, Failure);
        return;
    }

    const float RetryInterval = FMath::Max(0.01f, DistributionRetryInterval);
    GetWorld()->GetTimerManager().SetTimer(DeferredDistributionTimerHandle, this, &UEDInventoryComponent::ProcessDeferredDistribution, RetryInterval, true);
}

void UEDInventoryComponent::ProcessDeferredDistribution()
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || bDistributionCompleted || !GetWorld())
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(DeferredDistributionTimerHandle);
        }
        return;
    }

    TArray<UEDInventoryComponent*> ReadyTargets;
    bool bHasPendingTargets = false;
    GatherDistributionTargets(ReadyTargets, bHasPendingTargets);

    const bool bSourceReady = IsReadyForDistribution();
    const float Elapsed = GetWorld()->GetTimeSeconds() - DeferredDistributionStartTime;
    const bool bTimedOut = Elapsed >= FMath::Max(0.0f, DistributionTargetWaitTimeout);

    if (!bTimedOut && (!bSourceReady || bHasPendingTargets))
   {
        return;
    }

    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    bDistributionCompleted = ExecuteDistribution(ReadyTargets, Failure);
    GetWorld()->GetTimerManager().ClearTimer(DeferredDistributionTimerHandle);
}

void UEDInventoryComponent::ServerRequestMoveItemBetweenSlots_Implementation(int32 FromSlotIndex, int32 ToSlotIndex)
{
    RequestMoveItemBetweenSlots(FromSlotIndex, ToSlotIndex);
}

void UEDInventoryComponent::ServerRequestInitializeInventorySlots_Implementation()
{
    RequestInitializeInventorySlots();
}

void UEDInventoryComponent::ServerRequestTransferItemAuto_Implementation(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    RequestTransferItemAuto(FromInventory, ToInventory, FromSlotIndex, Quantity);
}

void UEDInventoryComponent::ServerRequestTransferItemToSlot_Implementation(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    RequestTransferItemToSlot(FromInventory, ToInventory, FromSlotIndex, ToSlotIndex, Quantity);
}

void UEDInventoryComponent::ServerRequestDropAllFromSlot_Implementation(int32 FromSlotIndex)
{
    RequestDropAllFromSlot(FromSlotIndex);
}

void UEDInventoryComponent::ServerRequestDropSingleFromSlot_Implementation(int32 FromSlotIndex)
{
    RequestDropSingleFromSlot(FromSlotIndex);
}

void UEDInventoryComponent::ServerRequestDropCountFromSlot_Implementation(int32 FromSlotIndex, int32 DropCount)
{
    RequestDropCountFromSlot(FromSlotIndex, DropCount);
}

void UEDInventoryComponent::ServerRequestPickupDroppedItem_Implementation(AEDDroppedItemActor* DroppedItemActor)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    RequestPickupDroppedItemDetailed(DroppedItemActor, Failure);
}

void UEDInventoryComponent::ServerRequestEquipItemFromSlot_Implementation(int32 FromSlotIndex, EEDEquippableType TargetSlotType)
{
    RequestEquipItemFromSlot(FromSlotIndex, TargetSlotType);
}

void UEDInventoryComponent::ServerRequestUnequipTopArmor_Implementation()
{
    RequestUnequipTopArmor();
}

void UEDInventoryComponent::ServerRequestUnequipBottomArmor_Implementation()
{
    RequestUnequipBottomArmor();
}

void UEDInventoryComponent::ServerRequestCraftItem_Implementation(FName RecipeId)
{
    RequestCraftItem(RecipeId);
}

void UEDInventoryComponent::ServerRequestConsumeItemAtSlot_Implementation(int32 SlotIndex)
{
    RequestConsumeItemAtSlot(SlotIndex);
}

void UEDInventoryComponent::ServerRequestEnsureDefaultEquipment_Implementation()
{
    RequestEnsureDefaultEquipment();
}

void UEDInventoryComponent::ServerRequestAddItemAuto_Implementation(FPrimaryAssetId ItemId, int32 Quantity)
{
    RequestAddItemAuto(ItemId, Quantity);
}

void UEDInventoryComponent::ServerRequestAddItemToSlot_Implementation(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex)
{
    RequestAddItemToSlot(ItemId, Quantity, SlotIndex);
}

void UEDInventoryComponent::ServerRequestInitializeRandomLoot_Implementation(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    RequestInitializeRandomLootDetailed(RollCount, MinLootIndex, MaxLootIndex, Seed, Failure);
}

void UEDInventoryComponent::ServerRequestDistributeInventoryToTargets_Implementation()
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    RequestDistributeInventoryToTargetsDetailed(Failure);
}

void UEDInventoryComponent::OnRep_InventorySlots()
{
    UE_LOG(LogTemp, Warning, TEXT("InventoryRep: Owner=%s Slots=%d"),
        GetOwner() ? *GetOwner()->GetName() : TEXT("None"),
        InventorySlots.Num());
    OnInventoryChanged.Broadcast();
}

void UEDInventoryComponent::OnRep_WeaponSlot()
{
    OnInventoryChanged.Broadcast();
}

void UEDInventoryComponent::OnRep_TopArmorSlot()
{
    OnInventoryChanged.Broadcast();
}

void UEDInventoryComponent::OnRep_BottomArmorSlot()
{
    OnInventoryChanged.Broadcast();
}

void UEDInventoryComponent::HandleInventoryChangedInternal()
{
    if (bAutoRefreshCraftableRecipesCache)
    {
        RefreshCraftableRecipesCache();
    }
}

void UEDInventoryComponent::HandleDropRequestSpawnWorldItem(const FEDInventoryDropRequest& DropRequest)
{
    if (!GetOwner() || !GetOwner()->HasAuthority() || !bSpawnDroppedItemActor)
    {
        return;
    }

    if (!DropRequest.Item.IsValid())
    {
        return;
    }

    const FVector SpawnOrigin = GetOwner()->GetActorLocation() + DroppedItemSpawnOffset;
    TrySpawnOrMergeDroppedItem(DropRequest.Item, SpawnOrigin);
}

bool UEDInventoryComponent::TrySpawnOrMergeDroppedItem(const FEDInventoryItemHandle& ItemHandle, const FVector& SpawnOrigin)
{
    if (!GetWorld() || !ItemHandle.IsValid())
    {
        return false;
    }

    if (DroppedItemMergeRadius > 0.0f)
    {
        const float MergeDistanceSq = FMath::Square(DroppedItemMergeRadius);
        for (TActorIterator<AEDDroppedItemActor> It(GetWorld()); It; ++It)
        {
            AEDDroppedItemActor* ExistingDrop = *It;
            if (!ExistingDrop)
            {
                continue;
            }

            if (FVector::DistSquared(ExistingDrop->GetActorLocation(), SpawnOrigin) > MergeDistanceSq)
            {
                continue;
            }

            if (ExistingDrop->TryMergeDroppedItem(ItemHandle.ItemId, ItemHandle.Quantity))
            {
                return true;
            }
        }
    }

    UClass* SpawnClass = DroppedItemActorClass ? DroppedItemActorClass.Get() : AEDDroppedItemActor::StaticClass();
    if (!SpawnClass)
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEDDroppedItemActor* NewDropActor = GetWorld()->SpawnActor<AEDDroppedItemActor>(SpawnClass, SpawnOrigin, FRotator::ZeroRotator, SpawnParams);
    if (!NewDropActor)
    {
        return false;
    }

    NewDropActor->InitializeDroppedItem(ItemHandle.ItemId, ItemHandle.Quantity);
    return true;
}

void UEDInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UEDInventoryComponent, MaxInventorySlots);
    DOREPLIFETIME(UEDInventoryComponent, InventorySlots);
    DOREPLIFETIME(UEDInventoryComponent, WeaponSlot);
    DOREPLIFETIME(UEDInventoryComponent, TopArmorSlot);
    DOREPLIFETIME(UEDInventoryComponent, BottomArmorSlot);
}

