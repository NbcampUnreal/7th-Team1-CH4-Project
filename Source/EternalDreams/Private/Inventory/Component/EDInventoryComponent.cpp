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

void UEDInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UEDInventoryComponent, MaxInventorySlots);
    DOREPLIFETIME(UEDInventoryComponent, InventorySlots);
    DOREPLIFETIME(UEDInventoryComponent, WeaponSlot);
    DOREPLIFETIME(UEDInventoryComponent, TopArmorSlot);
    DOREPLIFETIME(UEDInventoryComponent, BottomArmorSlot);
}
