#include "Inventory/BP/InventoryBlueprintLibrary.h"

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

