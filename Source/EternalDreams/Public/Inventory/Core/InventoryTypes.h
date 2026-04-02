#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Item/Core/ItemTypes.h"
#include "InventoryTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EInventoryDropReason : uint8
{
    UserRequested,
    UnequipNoSpace,
    CraftSwap,
    System
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FInventorySlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FInventoryItemHandle Item;

    bool IsEmpty() const
    {
        return !Item.IsValid();
    }
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEquipmentSlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EEquippableType SlotType = EEquippableType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FInventoryItemHandle EquippedItem;
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FInventoryDropRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FInventoryItemHandle Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TObjectPtr<AActor> SourceOwner = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    EInventoryDropReason Reason = EInventoryDropReason::UserRequested;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    FGameplayTagContainer ContextTags;
};
