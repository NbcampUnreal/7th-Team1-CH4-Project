#include "Inventory/Component/EDInventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Actor.h"
#include "Inventory/GAS/EDInventoryGASBridge.h"
#include "Inventory/System/EDInventoryCraftService.h"
#include "Inventory/System/EDInventoryEquipmentService.h"
#include "Inventory/System/EDInventoryTransferService.h"
#include "Inventory/System/EDInventoryValidationService.h"
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
}

UEDInventoryComponent::UEDInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    WeaponSlot.SlotType = EEDEquippableType::Weapon;
    TopArmorSlot.SlotType = EEDEquippableType::TopArmor;
    BottomArmorSlot.SlotType = EEDEquippableType::BottomArmor;
}

void UEDInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    OnInventoryChanged.AddUniqueDynamic(this, &UEDInventoryComponent::HandleInventoryChangedInternal);

    if (GetOwner() && GetOwner()->HasAuthority() && InventorySlots.Num() != MaxInventorySlots)
    {
        InitializeInventorySlots();
    }

    if (bGiveDefaultWeaponOnBeginPlay)
    {
        EnsureDefaultEquipment();
    }

    if (GetOwner() && GetOwner()->HasAuthority() && bUseEquipmentSlots)
    {
        SyncEquipEffectForSlot(EEDEquippableType::Weapon);
        SyncEquipEffectForSlot(EEDEquippableType::TopArmor);
        SyncEquipEffectForSlot(EEDEquippableType::BottomArmor);
    }

    if (GetOwner() && GetOwner()->HasAuthority() && bAutoInitializeRandomLootOnBeginPlay && !bRandomLootInitialized)
    {
        EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
        TryInitializeRandomLootDetailed(RandomLootRollCount, RandomLootMinIndex, RandomLootMaxIndex, RandomLootSeed, Failure);
    }

    RefreshCraftableRecipesCache();
}

void UEDInventoryComponent::InitializeInventorySlots()
{
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
        return false;
    }

    if (EffectHandle->IsValid())
    {
        ASC->RemoveActiveGameplayEffect(*EffectHandle);
        EffectHandle->Invalidate();
    }

    if (!EquipmentSlot->EquippedItem.IsValid())
    {
        return true;
    }

    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Component(EquipmentSlot->EquippedItem.ItemId);
    if (!ItemData || !ItemData->EquipEffectClass)
    {
        return true;
    }

    *EffectHandle = UEDInventoryGASBridge::ApplyEquipEffectWithHandle(GetOwner(), ASC, ItemData);
    if (!EffectHandle->WasSuccessfullyApplied())
    {
        EffectHandle->Invalidate();
        return false;
    }

    return true;
}

bool UEDInventoryComponent::TryMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryMoveItemBetweenSlots(FromSlotIndex, ToSlotIndex);
        return true;
    }

    const bool bSucceeded = FEDInventoryTransferService::MoveOrSwap(this, FromSlotIndex, ToSlotIndex);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::TryTransferItemAuto(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return TryTransferItemAutoDetailed(ToInventory, FromSlotIndex, Quantity, Failure);
}

bool UEDInventoryComponent::TryTransferItemAutoDetailed(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryTransferItemAuto(ToInventory, FromSlotIndex, Quantity);
        return true;
    }

    const bool bSucceeded = FEDInventoryTransferService::TransferAuto(this, ToInventory, FromSlotIndex, Quantity, &OutFailure);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
        if (ToInventory && ToInventory != this)
        {
            ToInventory->OnInventoryChanged.Broadcast();
        }
    }

    return bSucceeded;
}

bool UEDInventoryComponent::TryTransferItemToSlot(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return TryTransferItemToSlotDetailed(ToInventory, FromSlotIndex, ToSlotIndex, Quantity, Failure);
}

bool UEDInventoryComponent::TryTransferItemToSlotDetailed(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryTransferItemToSlot(ToInventory, FromSlotIndex, ToSlotIndex, Quantity);
        return true;
    }

    const bool bSucceeded = FEDInventoryTransferService::TransferToSlot(this, ToInventory, FromSlotIndex, ToSlotIndex, Quantity, &OutFailure);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
        if (ToInventory && ToInventory != this)
        {
            ToInventory->OnInventoryChanged.Broadcast();
        }
    }

    return bSucceeded;
}

bool UEDInventoryComponent::TryDropAllFromSlot(int32 FromSlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryDropAllFromSlot(FromSlotIndex);
        return true;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex) || InventorySlots[FromSlotIndex].IsEmpty())
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

bool UEDInventoryComponent::TryDropSingleFromSlot(int32 FromSlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryDropSingleFromSlot(FromSlotIndex);
        return true;
    }

    if (!InventorySlots.IsValidIndex(FromSlotIndex) || InventorySlots[FromSlotIndex].IsEmpty())
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

bool UEDInventoryComponent::TryEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryEquipItemFromSlot(FromSlotIndex, TargetSlotType);
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

bool UEDInventoryComponent::TryUnequipTopArmor()
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryUnequipTopArmor();
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

bool UEDInventoryComponent::TryUnequipBottomArmor()
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryUnequipBottomArmor();
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

bool UEDInventoryComponent::TryCraftItem(FName RecipeId)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return TryCraftItemDetailed(RecipeId, Failure);
}

bool UEDInventoryComponent::TryCraftItemDetailed(FName RecipeId, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryCraftItem(RecipeId);
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

bool UEDInventoryComponent::CanCraftRecipeByRowId(FName RecipeRowId) const
{
    if (!CraftingRecipeTable || RecipeRowId.IsNone())
    {
        return false;
    }

    const FEDCraftingRecipeRow* RecipeRow = CraftingRecipeTable->FindRow<FEDCraftingRecipeRow>(RecipeRowId, TEXT("CanCraftRecipeByRowId"));
    if (!RecipeRow)
    {
        return false;
    }

    return FEDInventoryCraftService::CanCraftRecipe(this, *RecipeRow, nullptr);
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

bool UEDInventoryComponent::TryCraftFirstCachedRecipe()
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

    return TryCraftItem(RecipeRowId);
}

bool UEDInventoryComponent::TryConsumeItemAtSlot(int32 SlotIndex)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return TryConsumeItemAtSlotDetailed(SlotIndex, Failure);
}

bool UEDInventoryComponent::TryConsumeItemAtSlotDetailed(int32 SlotIndex, EEDInventoryActionFailure& OutFailure)
{
    OutFailure = EEDInventoryActionFailure::None;

    if (!GetOwner())
    {
        OutFailure = EEDInventoryActionFailure::InvalidInventory;
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryConsumeItemAtSlot(SlotIndex);
        return true;
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

bool UEDInventoryComponent::EnsureDefaultEquipment()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return false;
    }

    const bool bSucceeded = FEDInventoryEquipmentService::EnsureDefaultWeapon(this);
    if (bSucceeded)
    {
        SyncEquipEffectForSlot(EEDEquippableType::Weapon);
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UEDInventoryComponent::TryAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return TryAddItemAutoDetailed(ItemId, Quantity, Failure);
}

bool UEDInventoryComponent::TryAddItemAutoDetailed(FPrimaryAssetId ItemId, int32 Quantity, EEDInventoryActionFailure& OutFailure)
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

    if (!GetOwner()->HasAuthority())
    {
        ServerTryAddItemAuto(ItemId, Quantity);
        return true;
    }

    if (!AddItemAuto_Component(this, ItemId, Quantity))
    {
        OutFailure = EEDInventoryActionFailure::NoSpace;
        return false;
    }

    OnInventoryChanged.Broadcast();
    return true;
}

bool UEDInventoryComponent::TryAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return TryAddItemToSlotDetailed(ItemId, Quantity, SlotIndex, Failure);
}

bool UEDInventoryComponent::TryAddItemToSlotDetailed(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure& OutFailure)
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

    if (!GetOwner()->HasAuthority())
    {
        ServerTryAddItemToSlot(ItemId, Quantity, SlotIndex);
        return true;
    }

    if (!AddItemToSlot_Component(this, ItemId, Quantity, SlotIndex, &OutFailure))
    {
        return false;
    }

    OnInventoryChanged.Broadcast();
    return true;
}

bool UEDInventoryComponent::TryInitializeRandomLoot()
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    return TryInitializeRandomLootDetailed(RandomLootRollCount, RandomLootMinIndex, RandomLootMaxIndex, RandomLootSeed, Failure);
}

bool UEDInventoryComponent::TryInitializeRandomLootDetailed(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed, EEDInventoryActionFailure& OutFailure)
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
        ServerTryInitializeRandomLoot(RollCount, MinLootIndex, MaxLootIndex, Seed);
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

    FRandomStream RandomStream;
    if (Seed != 0)
    {
        RandomStream.Initialize(Seed);
    }
    else
    {
        RandomStream.GenerateNewSeed();
    }

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

void UEDInventoryComponent::ServerTryMoveItemBetweenSlots_Implementation(int32 FromSlotIndex, int32 ToSlotIndex)
{
    TryMoveItemBetweenSlots(FromSlotIndex, ToSlotIndex);
}

void UEDInventoryComponent::ServerTryTransferItemAuto_Implementation(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    TryTransferItemAuto(ToInventory, FromSlotIndex, Quantity);
}

void UEDInventoryComponent::ServerTryTransferItemToSlot_Implementation(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    TryTransferItemToSlot(ToInventory, FromSlotIndex, ToSlotIndex, Quantity);
}

void UEDInventoryComponent::ServerTryDropAllFromSlot_Implementation(int32 FromSlotIndex)
{
    TryDropAllFromSlot(FromSlotIndex);
}

void UEDInventoryComponent::ServerTryDropSingleFromSlot_Implementation(int32 FromSlotIndex)
{
    TryDropSingleFromSlot(FromSlotIndex);
}

void UEDInventoryComponent::ServerTryEquipItemFromSlot_Implementation(int32 FromSlotIndex, EEDEquippableType TargetSlotType)
{
    TryEquipItemFromSlot(FromSlotIndex, TargetSlotType);
}

void UEDInventoryComponent::ServerTryUnequipTopArmor_Implementation()
{
    TryUnequipTopArmor();
}

void UEDInventoryComponent::ServerTryUnequipBottomArmor_Implementation()
{
    TryUnequipBottomArmor();
}

void UEDInventoryComponent::ServerTryCraftItem_Implementation(FName RecipeId)
{
    TryCraftItem(RecipeId);
}

void UEDInventoryComponent::ServerTryConsumeItemAtSlot_Implementation(int32 SlotIndex)
{
    TryConsumeItemAtSlot(SlotIndex);
}

void UEDInventoryComponent::ServerTryAddItemAuto_Implementation(FPrimaryAssetId ItemId, int32 Quantity)
{
    TryAddItemAuto(ItemId, Quantity);
}

void UEDInventoryComponent::ServerTryAddItemToSlot_Implementation(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex)
{
    TryAddItemToSlot(ItemId, Quantity, SlotIndex);
}

void UEDInventoryComponent::ServerTryInitializeRandomLoot_Implementation(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed)
{
    EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
    TryInitializeRandomLootDetailed(RollCount, MinLootIndex, MaxLootIndex, Seed, Failure);
}

void UEDInventoryComponent::OnRep_InventorySlots()
{
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

void UEDInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UEDInventoryComponent, MaxInventorySlots);
    DOREPLIFETIME(UEDInventoryComponent, InventorySlots);
    DOREPLIFETIME(UEDInventoryComponent, WeaponSlot);
    DOREPLIFETIME(UEDInventoryComponent, TopArmorSlot);
    DOREPLIFETIME(UEDInventoryComponent, BottomArmorSlot);
}
