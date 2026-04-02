#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryGASBridge.generated.h"

class AActor;
class UAbilitySystemComponent;
class UInventoryItemDataAsset;

UCLASS()
class ETERNALDREAMS_API UInventoryGASBridge : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory|GAS")
    static bool ApplyConsumableEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UInventoryItemDataAsset* ItemData);

    UFUNCTION(BlueprintCallable, Category = "Inventory|GAS")
    static bool ApplyEquipEffect(AActor* SourceActor, UAbilitySystemComponent* TargetASC, const UInventoryItemDataAsset* ItemData);
};

