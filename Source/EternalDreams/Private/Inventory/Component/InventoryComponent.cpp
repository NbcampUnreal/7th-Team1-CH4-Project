#include "Inventory/Component/InventoryComponent.h"

#include "Inventory/System/InventoryCraftService.h"
#include "Inventory/System/InventoryEquipmentService.h"
#include "Inventory/System/InventoryTransferService.h"
#include "Inventory/System/InventoryValidationService.h"
#include "Net/UnrealNetwork.h"

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
    return false;
}

bool UInventoryComponent::TryTransferItemAuto(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
    return false;
}

bool UInventoryComponent::TryTransferItemToSlot(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
    return false;
}

bool UInventoryComponent::TryDropAllFromSlot(int32 FromSlotIndex)
{
    return false;
}

bool UInventoryComponent::TryDropSingleFromSlot(int32 FromSlotIndex)
{
    return false;
}

bool UInventoryComponent::TryEquipItemFromSlot(int32 FromSlotIndex, EEquippableType TargetSlotType)
{
    return false;
}

bool UInventoryComponent::TryUnequipTopArmor()
{
    return false;
}

bool UInventoryComponent::TryUnequipBottomArmor()
{
    return false;
}

bool UInventoryComponent::TryCraftItem(FName RecipeId)
{
    return false;
}

bool UInventoryComponent::TryConsumeItemAtSlot(int32 SlotIndex)
{
    return false;
}

bool UInventoryComponent::EnsureDefaultEquipment()
{
    return false;
}

void UInventoryComponent::ServerTryMoveItemBetweenSlots_Implementation(int32 FromSlotIndex, int32 ToSlotIndex)
{
}

void UInventoryComponent::ServerTryTransferItemAuto_Implementation(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity)
{
}

void UInventoryComponent::ServerTryTransferItemToSlot_Implementation(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity)
{
}

void UInventoryComponent::ServerTryDropAllFromSlot_Implementation(int32 FromSlotIndex)
{
}

void UInventoryComponent::ServerTryDropSingleFromSlot_Implementation(int32 FromSlotIndex)
{
}

void UInventoryComponent::ServerTryEquipItemFromSlot_Implementation(int32 FromSlotIndex, EEquippableType TargetSlotType)
{
}

void UInventoryComponent::ServerTryUnequipTopArmor_Implementation()
{
}

void UInventoryComponent::ServerTryUnequipBottomArmor_Implementation()
{
}

void UInventoryComponent::ServerTryCraftItem_Implementation(FName RecipeId)
{
}

void UInventoryComponent::ServerTryConsumeItemAtSlot_Implementation(int32 SlotIndex)
{
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

