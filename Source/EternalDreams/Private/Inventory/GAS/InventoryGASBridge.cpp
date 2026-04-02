#include "Inventory/GAS/InventoryGASBridge.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Item/Data/InventoryItemDataAsset.h"

namespace
{
bool ApplyEffectFromItemData(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UInventoryItemDataAsset* ItemData)
{
    if (!TargetASC || !ItemData || !ItemData->ConsumableEffectClass)
    {
        return false;
    }

    FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
    if (SourceActor)
    {
        EffectContext.AddSourceObject(SourceActor);
    }

    const FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(ItemData->ConsumableEffectClass, 1.0f, EffectContext);
    if (!EffectSpec.IsValid())
    {
        return false;
    }

    const FActiveGameplayEffectHandle AppliedHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
    return AppliedHandle.WasSuccessfullyApplied();
}
}

bool UInventoryGASBridge::ApplyConsumableEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UInventoryItemDataAsset* ItemData)
{
    return ApplyEffectFromItemData(SourceActor, TargetASC, ItemData);
}

bool UInventoryGASBridge::ApplyEquipEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UInventoryItemDataAsset* ItemData)
{
    return ApplyEffectFromItemData(SourceActor, TargetASC, ItemData);
}
