#include "Inventory/System/InventoryValidationService.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Item/Data/InventoryItemDataAsset.h"

namespace
{
void SetFailure(EInventoryActionFailure* OutFailure, EInventoryActionFailure Failure)
{
    if (OutFailure)
    {
        *OutFailure = Failure;
    }
}
}

bool FInventoryValidationService::IsSlotIndexValid(const UInventoryComponent* InventoryComponent, int32 SlotIndex)
{
    return InventoryComponent && InventoryComponent->InventorySlots.IsValidIndex(SlotIndex);
}

bool FInventoryValidationService::CanEquipToSlot(const UInventoryItemDataAsset* ItemData, EEquippableType TargetSlot)
{
    if (!ItemData)
    {
        return false;
    }

    if (ItemData->ItemType != EInventoryItemType::Equippable)
    {
        return false;
    }

    return ItemData->EquippableType == TargetSlot;
}

bool FInventoryValidationService::CanConsumeItem(const UInventoryItemDataAsset* ItemData, const AActor* OwnerActor, EInventoryActionFailure* OutFailure)
{
    SetFailure(OutFailure, EInventoryActionFailure::None);

    if (!ItemData)
    {
        SetFailure(OutFailure, EInventoryActionFailure::MissingData);
        return false;
    }

    if (ItemData->ItemType != EInventoryItemType::Consumable)
    {
        SetFailure(OutFailure, EInventoryActionFailure::NotConsumable);
        return false;
    }

    if (!OwnerActor)
    {
        SetFailure(OutFailure, EInventoryActionFailure::InvalidInventory);
        return false;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(const_cast<AActor*>(OwnerActor));
    if (!ASC)
    {
        return true;
    }

    const FGameplayTag HealthConsumableTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.Consumable.Health")), false);
    const FGameplayTag HealthFullTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Health.Full")), false);

    if (HealthConsumableTag.IsValid() && HealthFullTag.IsValid() && ItemData->ItemTags.HasTag(HealthConsumableTag) && ASC->HasMatchingGameplayTag(HealthFullTag))
    {
        SetFailure(OutFailure, EInventoryActionFailure::HealthAlreadyFull);
        return false;
    }

    return true;
}
