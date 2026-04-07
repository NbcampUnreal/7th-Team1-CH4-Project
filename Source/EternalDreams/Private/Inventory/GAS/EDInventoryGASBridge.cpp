#include "Inventory/GAS/EDInventoryGASBridge.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Item/Data/EDInventoryItemDataAsset.h"

namespace
{
FActiveGameplayEffectHandle ApplyEffectClass(AActor* SourceActor, UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> EffectClass)
{
    if (!TargetASC || !EffectClass)
    {
        return FActiveGameplayEffectHandle();
    }

    FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
    if (SourceActor)
    {
        EffectContext.AddSourceObject(SourceActor);
    }

    const FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
    if (!EffectSpec.IsValid())
    {
        return FActiveGameplayEffectHandle();
    }

    return TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
}
}

bool UEDInventoryGASBridge::ApplyConsumableEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UEDInventoryItemDataAsset* ItemData)
{
    if (!ItemData)
    {
        return false;
    }

    return ApplyEffectClass(SourceActor, TargetASC, ItemData->ConsumableEffectClass).WasSuccessfullyApplied();
}

bool UEDInventoryGASBridge::ApplyEquipEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UEDInventoryItemDataAsset* ItemData)
{
    return ApplyEquipEffectWithHandle(SourceActor, TargetASC, ItemData).WasSuccessfullyApplied();
}

FActiveGameplayEffectHandle UEDInventoryGASBridge::ApplyEquipEffectWithHandle(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UEDInventoryItemDataAsset* ItemData)
{
    if (!ItemData)
    {
        return FActiveGameplayEffectHandle();
    }

    return ApplyEffectClass(SourceActor, TargetASC, ItemData->EquipEffectClass);
}
