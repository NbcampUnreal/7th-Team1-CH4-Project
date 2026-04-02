#include "Inventory/GAS/InventoryGASBridge.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Item/Data/InventoryItemDataAsset.h"

bool UInventoryGASBridge::ApplyConsumableEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UInventoryItemDataAsset* ItemData)
{
    return false;
}

bool UInventoryGASBridge::ApplyEquipEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UInventoryItemDataAsset* ItemData)
{
    return false;
}
