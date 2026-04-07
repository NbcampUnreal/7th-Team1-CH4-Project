#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEffectTypes.h"
#include "EDInventoryGASBridge.generated.h"

class AActor;
class UAbilitySystemComponent;
class UEDInventoryItemDataAsset;

UCLASS()
class ETERNALDREAMS_API UEDInventoryGASBridge : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory|GAS")
    static bool ApplyConsumableEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UEDInventoryItemDataAsset* ItemData);

    UFUNCTION(BlueprintCallable, Category = "Inventory|GAS")
    static bool ApplyEquipEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UEDInventoryItemDataAsset* ItemData);

    static FActiveGameplayEffectHandle ApplyEquipEffectWithHandle(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UEDInventoryItemDataAsset* ItemData);
};
