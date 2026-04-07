#include "Inventory/BP/EDInventoryBlueprintLibrary.h"

#include "Internationalization/Text.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/Interface/EDInventoryProviderInterface.h"

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
