#include "Inventory/BP/InventoryBlueprintLibrary.h"

#include "Internationalization/Text.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Inventory/Interface/InventoryProviderInterface.h"

UInventoryComponent* UInventoryBlueprintLibrary::GetInventoryComponentFromActor(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }

    if (Actor->GetClass()->ImplementsInterface(UInventoryProviderInterface::StaticClass()))
    {
        return IInventoryProviderInterface::Execute_GetInventoryComponent(Actor);
    }

    return Actor->FindComponentByClass<UInventoryComponent>();
}

bool UInventoryBlueprintLibrary::IsValidInventorySlotIndex(const UInventoryComponent* InventoryComponent, int32 SlotIndex)
{
    return InventoryComponent && InventoryComponent->InventorySlots.IsValidIndex(SlotIndex);
}

bool UInventoryBlueprintLibrary::IsInventoryActionSuccess(EInventoryActionFailure Failure)
{
    return Failure == EInventoryActionFailure::None;
}

FText UInventoryBlueprintLibrary::GetInventoryActionFailureText(EInventoryActionFailure Failure)
{
    switch (Failure)
    {
    case EInventoryActionFailure::None:
        return NSLOCTEXT("Inventory", "FailureNone", "No error");
    case EInventoryActionFailure::InvalidInventory:
        return NSLOCTEXT("Inventory", "FailureInvalidInventory", "Invalid inventory");
    case EInventoryActionFailure::InvalidSlot:
        return NSLOCTEXT("Inventory", "FailureInvalidSlot", "Invalid slot");
    case EInventoryActionFailure::EmptySlot:
        return NSLOCTEXT("Inventory", "FailureEmptySlot", "Selected slot is empty");
    case EInventoryActionFailure::InvalidQuantity:
        return NSLOCTEXT("Inventory", "FailureInvalidQuantity", "Invalid quantity");
    case EInventoryActionFailure::SlotConflict:
        return NSLOCTEXT("Inventory", "FailureSlotConflict", "Slot conflict");
    case EInventoryActionFailure::NoSpace:
        return NSLOCTEXT("Inventory", "FailureNoSpace", "No space available");
    case EInventoryActionFailure::StackLimit:
        return NSLOCTEXT("Inventory", "FailureStackLimit", "Stack limit reached");
    case EInventoryActionFailure::MissingData:
        return NSLOCTEXT("Inventory", "FailureMissingData", "Missing item data");
    case EInventoryActionFailure::InvalidRecipe:
        return NSLOCTEXT("Inventory", "FailureInvalidRecipe", "Invalid recipe");
    case EInventoryActionFailure::MissingIngredient:
        return NSLOCTEXT("Inventory", "FailureMissingIngredient", "Missing ingredient");
    case EInventoryActionFailure::NotConsumable:
        return NSLOCTEXT("Inventory", "FailureNotConsumable", "Item is not consumable");
    case EInventoryActionFailure::HealthAlreadyFull:
        return NSLOCTEXT("Inventory", "FailureHealthAlreadyFull", "Health is already full");
    case EInventoryActionFailure::EffectApplyFailed:
        return NSLOCTEXT("Inventory", "FailureEffectApplyFailed", "Failed to apply effect");
    default:
        return NSLOCTEXT("Inventory", "FailureUnknown", "Unknown failure");
    }
}
