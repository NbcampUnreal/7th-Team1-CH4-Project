#include "Inventory/Component/InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Actor.h"
#include "Inventory/GAS/InventoryGASBridge.h"
#include "Inventory/System/InventoryCraftService.h"
#include "Inventory/System/InventoryEquipmentService.h"
#include "Inventory/System/InventoryTransferService.h"
#include "Inventory/System/InventoryValidationService.h"
#include "Item/Data/InventoryItemDataAsset.h"
#include "Net/UnrealNetwork.h"

namespace
{
const UInventoryItemDataAsset* ResolveItemData(const FPrimaryAssetId& ItemId)
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

    return Cast<UInventoryItemDataAsset>(ItemObject);
}
}

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);

    WeaponSlot.SlotType = EEquippableType::Weapon;
    TopArmorSlot.SlotType = EEquippableType::TopArmor;
    BottomArmorSlot.SlotType = EEquippableType::BottomArmor;
}

void UInventoryComponent::BeginPlay()
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
}

void UInventoryComponent::InitializeInventorySlots()
{
    if (MaxInventorySlots < 0)
    {
        MaxInventorySlots = 0;
    }

    InventorySlots.SetNum(MaxInventorySlots);
    OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::TryMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex)
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

    const bool bSucceeded = FInventoryTransferService::MoveOrSwap(this, FromSlotIndex, ToSlotIndex);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UInventoryComponent::TryTransferItemAuto(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryTransferItemAuto(ToInventory, FromSlotIndex, Quantity);
        return true;
    }

    const bool bSucceeded = FInventoryTransferService::TransferAuto(this, ToInventory, FromSlotIndex, Quantity);
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

bool UInventoryComponent::TryTransferItemToSlot(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryTransferItemToSlot(ToInventory, FromSlotIndex, ToSlotIndex, Quantity);
        return true;
    }

    const bool bSucceeded = FInventoryTransferService::TransferToSlot(this, ToInventory, FromSlotIndex, ToSlotIndex, Quantity);
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

bool UInventoryComponent::TryDropAllFromSlot(int32 FromSlotIndex)
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

    FInventoryDropRequest DropRequest;
    DropRequest.Item = InventorySlots[FromSlotIndex].Item;
    DropRequest.SourceOwner = GetOwner();
    DropRequest.Reason = EInventoryDropReason::UserRequested;

    InventorySlots[FromSlotIndex].Item = FInventoryItemHandle();
    OnInventoryDropRequested.Broadcast(DropRequest);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::TryDropSingleFromSlot(int32 FromSlotIndex)
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

    FInventoryDropRequest DropRequest;
    DropRequest.Item.ItemId = InventorySlots[FromSlotIndex].Item.ItemId;
    DropRequest.Item.Quantity = 1;
    DropRequest.SourceOwner = GetOwner();
    DropRequest.Reason = EInventoryDropReason::UserRequested;

    InventorySlots[FromSlotIndex].Item.Quantity -= 1;
    if (InventorySlots[FromSlotIndex].Item.Quantity <= 0)
    {
        InventorySlots[FromSlotIndex].Item = FInventoryItemHandle();
    }

    OnInventoryDropRequested.Broadcast(DropRequest);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::TryEquipItemFromSlot(int32 FromSlotIndex, EEquippableType TargetSlotType)
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

    const bool bSucceeded = FInventoryEquipmentService::EquipFromSlot(this, FromSlotIndex, TargetSlotType);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UInventoryComponent::TryUnequipTopArmor()
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

    const bool bSucceeded = FInventoryEquipmentService::UnequipTopArmor(this);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UInventoryComponent::TryUnequipBottomArmor()
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

    const bool bSucceeded = FInventoryEquipmentService::UnequipBottomArmor(this);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UInventoryComponent::TryCraftItem(FName RecipeId)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryCraftItem(RecipeId);
        return true;
    }

    const bool bSucceeded = FInventoryCraftService::TryCraftByRecipeId(this, RecipeId);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

bool UInventoryComponent::TryConsumeItemAtSlot(int32 SlotIndex)
{
    if (!GetOwner())
    {
        return false;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerTryConsumeItemAtSlot(SlotIndex);
        return true;
    }

    if (!InventorySlots.IsValidIndex(SlotIndex) || InventorySlots[SlotIndex].IsEmpty())
    {
        return false;
    }

    const UInventoryItemDataAsset* ItemData = ResolveItemData(InventorySlots[SlotIndex].Item.ItemId);
    if (!FInventoryValidationService::CanConsumeItem(ItemData, GetOwner()))
    {
        return false;
    }

    if (ItemData && ItemData->ConsumableEffectClass)
    {
        UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
        if (!UInventoryGASBridge::ApplyConsumableEffect(GetOwner(), ASC, ItemData))
        {
            return false;
        }
    }

    InventorySlots[SlotIndex].Item.Quantity -= 1;
    if (InventorySlots[SlotIndex].Item.Quantity <= 0)
    {
        InventorySlots[SlotIndex].Item = FInventoryItemHandle();
    }

    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::EnsureDefaultEquipment()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return false;
    }

    const bool bSucceeded = FInventoryEquipmentService::EnsureDefaultWeapon(this);
    if (bSucceeded)
    {
        OnInventoryChanged.Broadcast();
    }

    return bSucceeded;
}

void UInventoryComponent::ServerTryMoveItemBetweenSlots_Implementation(int32 FromSlotIndex, int32 ToSlotIndex)
{
    TryMoveItemBetweenSlots(FromSlotIndex, ToSlotIndex);
}

void UInventoryComponent::ServerTryTransferItemAuto_Implementation(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    TryTransferItemAuto(ToInventory, FromSlotIndex, Quantity);
}

void UInventoryComponent::ServerTryTransferItemToSlot_Implementation(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    TryTransferItemToSlot(ToInventory, FromSlotIndex, ToSlotIndex, Quantity);
}

void UInventoryComponent::ServerTryDropAllFromSlot_Implementation(int32 FromSlotIndex)
{
    TryDropAllFromSlot(FromSlotIndex);
}

void UInventoryComponent::ServerTryDropSingleFromSlot_Implementation(int32 FromSlotIndex)
{
    TryDropSingleFromSlot(FromSlotIndex);
}

void UInventoryComponent::ServerTryEquipItemFromSlot_Implementation(int32 FromSlotIndex, EEquippableType TargetSlotType)
{
    TryEquipItemFromSlot(FromSlotIndex, TargetSlotType);
}

void UInventoryComponent::ServerTryUnequipTopArmor_Implementation()
{
    TryUnequipTopArmor();
}

void UInventoryComponent::ServerTryUnequipBottomArmor_Implementation()
{
    TryUnequipBottomArmor();
}

void UInventoryComponent::ServerTryCraftItem_Implementation(FName RecipeId)
{
    TryCraftItem(RecipeId);
}

void UInventoryComponent::ServerTryConsumeItemAtSlot_Implementation(int32 SlotIndex)
{
    TryConsumeItemAtSlot(SlotIndex);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInventoryComponent, MaxInventorySlots);
    DOREPLIFETIME(UInventoryComponent, InventorySlots);
    DOREPLIFETIME(UInventoryComponent, WeaponSlot);
    DOREPLIFETIME(UInventoryComponent, TopArmorSlot);
    DOREPLIFETIME(UInventoryComponent, BottomArmorSlot);
}
