#include "Inventory/System/EDInventoryValidationService.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"

namespace
{
void SetFailure_Validation(EEDInventoryActionFailure* OutFailure, EEDInventoryActionFailure Failure)
{
    if (OutFailure)
    {
        *OutFailure = Failure;
    }
}
}

bool FEDInventoryValidationService::IsSlotIndexValid(const UEDInventoryComponent* InventoryComponent, int32 SlotIndex)
{
    return InventoryComponent && InventoryComponent->InventorySlots.IsValidIndex(SlotIndex);
}

bool FEDInventoryValidationService::CanEquipToSlot(const UEDInventoryItemDataAsset* ItemData, EEDEquippableType TargetSlot)
{
    if (!ItemData)
    {
        return false;
    }

    if (ItemData->ItemType != EEDInventoryItemType::Equippable)
    {
        return false;
    }

    return ItemData->EquippableType == TargetSlot;
}

bool FEDInventoryValidationService::CanEquipToSkillSlot(const UEDInventoryItemDataAsset* ItemData)
{
    return ItemData && ItemData->ItemType == EEDInventoryItemType::Skill;
}

bool FEDInventoryValidationService::CanConsumeItem(const UEDInventoryItemDataAsset* ItemData, const AActor* OwnerActor, EEDInventoryActionFailure* OutFailure)
{
    SetFailure_Validation(OutFailure, EEDInventoryActionFailure::None);

    if (!ItemData)
    {
        SetFailure_Validation(OutFailure, EEDInventoryActionFailure::MissingData);
        return false;
    }

    if (ItemData->ItemType != EEDInventoryItemType::Consumable)
    {
        SetFailure_Validation(OutFailure, EEDInventoryActionFailure::NotConsumable);
        return false;
    }

    if (!OwnerActor)
    {
        SetFailure_Validation(OutFailure, EEDInventoryActionFailure::InvalidInventory);
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
        SetFailure_Validation(OutFailure, EEDInventoryActionFailure::HealthAlreadyFull);
        return false;
    }

    return true;
}
