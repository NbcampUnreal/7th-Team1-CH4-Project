#include "Inventory/System/InventoryCraftService.h"

#include "Engine/AssetManager.h"
#include "Inventory/Component/InventoryComponent.h"
#include "Item/Data/InventoryItemDataAsset.h"
#include "Item/Data/ItemDataRows.h"

namespace
{
const UInventoryItemDataAsset* ResolveItemData_Craft(const FPrimaryAssetId& ItemId)
{
    if (!ItemId.IsValid())
    {
        return nullptr;
    }

    UObject* ItemObject = UAssetManager::Get().GetPrimaryAssetObject(ItemId);
    if (!ItemObject)
    {
        const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
        if (AssetPath.IsValid())
        {
            ItemObject = AssetPath.TryLoad();
        }
    }

    return Cast<UInventoryItemDataAsset>(ItemObject);
}

int32 GetItemMaxStack_Craft(const FPrimaryAssetId& ItemId)
{
    const UInventoryItemDataAsset* ItemData = ResolveItemData_Craft(ItemId);
    return ItemData ? FMath::Max(1, ItemData->MaxStack) : 1;
}

void SetFailure_Craft(EInventoryActionFailure* OutFailure, EInventoryActionFailure Failure)
{
    if (OutFailure)
    {
        *OutFailure = Failure;
    }
}

int32 CountItemInInventory(const UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    int32 Count = 0;
    for (const FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (!Slot.IsEmpty() && Slot.Item.ItemId == ItemId)
        {
            Count += Slot.Item.Quantity;
        }
    }

    return Count;
}

bool CanStoreResultItem(const UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity)
{
    const int32 MaxStack = GetItemMaxStack_Craft(ItemId);
    int32 Capacity = 0;

    for (const FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Slot.IsEmpty())
        {
            Capacity += MaxStack;
        }
        else if (Slot.Item.ItemId == ItemId)
        {
            Capacity += FMath::Max(0, MaxStack - Slot.Item.Quantity);
        }

        if (Capacity >= Quantity)
        {
            return true;
        }
    }

    return false;
}

bool IsEquipmentResult(const UInventoryItemDataAsset* ItemData)
{
    return ItemData && ItemData->ItemType == EInventoryItemType::Equippable && ItemData->EquippableType != EEquippableType::None;
}

FEquipmentSlotData* GetEquipmentSlot_Craft(UInventoryComponent* InventoryComponent, EEquippableType SlotType)
{
    if (!InventoryComponent)
    {
        return nullptr;
    }

    switch (SlotType)
    {
    case EEquippableType::Weapon:
        return &InventoryComponent->WeaponSlot;
    case EEquippableType::TopArmor:
        return &InventoryComponent->TopArmorSlot;
    case EEquippableType::BottomArmor:
        return &InventoryComponent->BottomArmorSlot;
    default:
        return nullptr;
    }
}

int32 CountItemInEquipment(const UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    int32 Count = 0;

    const TArray<const FEquipmentSlotData*> EquipmentSlots =
    {
        &InventoryComponent->WeaponSlot,
        &InventoryComponent->TopArmorSlot,
        &InventoryComponent->BottomArmorSlot
    };

    for (const FEquipmentSlotData* Slot : EquipmentSlots)
    {
        if (Slot && Slot->EquippedItem.IsValid() && Slot->EquippedItem.ItemId == ItemId)
        {
            Count += Slot->EquippedItem.Quantity;
        }
    }

    return Count;
}

int32 CountItemTotal(const UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    return CountItemInInventory(InventoryComponent, ItemId) + CountItemInEquipment(InventoryComponent, ItemId);
}

int32 StoreItemToInventory(UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity)
{
    if (!InventoryComponent || !ItemId.IsValid() || Quantity <= 0)
    {
        return Quantity;
    }

    const int32 MaxStack = GetItemMaxStack_Craft(ItemId);
    int32 Remaining = Quantity;

    for (FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Remaining <= 0)
        {
            break;
        }

        if (!Slot.IsEmpty() && Slot.Item.ItemId == ItemId)
        {
            const int32 SpaceLeft = FMath::Max(0, MaxStack - Slot.Item.Quantity);
            const int32 AddAmount = FMath::Min(Remaining, SpaceLeft);
            if (AddAmount > 0)
            {
                Slot.Item.Quantity += AddAmount;
                Remaining -= AddAmount;
            }
        }
    }

    for (FInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (Remaining <= 0)
        {
            break;
        }

        if (Slot.IsEmpty())
        {
            const int32 AddAmount = FMath::Min(Remaining, MaxStack);
            Slot.Item.ItemId = ItemId;
            Slot.Item.Quantity = AddAmount;
            Remaining -= AddAmount;
        }
    }

    return Remaining;
}

void DropCraftedItem(UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity)
{
    if (!InventoryComponent || !ItemId.IsValid() || Quantity <= 0)
    {
        return;
    }

    FInventoryDropRequest DropRequest;
    DropRequest.Item.ItemId = ItemId;
    DropRequest.Item.Quantity = Quantity;
    DropRequest.SourceOwner = InventoryComponent->GetOwner();
    DropRequest.Reason = EInventoryDropReason::CraftSwap;
    InventoryComponent->OnInventoryDropRequested.Broadcast(DropRequest);
}

bool ConsumeIngredientFromEquipment(UInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32& InOutRemaining, bool& bOutUsedEquippedIngredient, EEquippableType& OutLastConsumedSlotType)
{
    if (!InventoryComponent || InOutRemaining <= 0)
    {
        return true;
    }

    const TArray<EEquippableType> ConsumeOrder =
    {
        EEquippableType::Weapon,
        EEquippableType::TopArmor,
        EEquippableType::BottomArmor
    };

    for (EEquippableType SlotType : ConsumeOrder)
    {
        if (InOutRemaining <= 0)
        {
            break;
        }

        FEquipmentSlotData* Slot = GetEquipmentSlot_Craft(InventoryComponent, SlotType);
        if (Slot && Slot->EquippedItem.IsValid() && Slot->EquippedItem.ItemId == ItemId)
        {
            const int32 ConsumeCount = FMath::Min(InOutRemaining, Slot->EquippedItem.Quantity);
            Slot->EquippedItem.Quantity -= ConsumeCount;
            InOutRemaining -= ConsumeCount;
            bOutUsedEquippedIngredient = true;
            OutLastConsumedSlotType = SlotType;

            if (Slot->EquippedItem.Quantity <= 0)
            {
                Slot->EquippedItem = FInventoryItemHandle();
            }
        }
    }

    return InOutRemaining <= 0;
}

bool ShouldAutoEquipCraftResult(UInventoryComponent* InventoryComponent, EEquippableType ResultSlotType, bool bUsedEquippedIngredient, EEquippableType LastConsumedSlotType)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    if (ResultSlotType == EEquippableType::Weapon)
    {
        return true;
    }

    FEquipmentSlotData* ResultSlot = GetEquipmentSlot_Craft(InventoryComponent, ResultSlotType);
    if (!ResultSlot)
    {
        return false;
    }

    if (bUsedEquippedIngredient && LastConsumedSlotType == ResultSlotType)
    {
        return true;
    }

    return !ResultSlot->EquippedItem.IsValid();
}

void EquipCraftResultOrFallback(UInventoryComponent* InventoryComponent, EEquippableType SlotType, const FPrimaryAssetId& ResultItemId)
{
    FEquipmentSlotData* TargetSlot = GetEquipmentSlot_Craft(InventoryComponent, SlotType);
    if (!InventoryComponent || !TargetSlot || !ResultItemId.IsValid())
    {
        return;
    }

    if (TargetSlot->EquippedItem.IsValid())
    {
        const FInventoryItemHandle PreviousEquippedItem = TargetSlot->EquippedItem;
        int32 RemainingPrevious = StoreItemToInventory(InventoryComponent, PreviousEquippedItem.ItemId, PreviousEquippedItem.Quantity);
        if (RemainingPrevious > 0)
        {
            DropCraftedItem(InventoryComponent, PreviousEquippedItem.ItemId, RemainingPrevious);
        }
    }

    TargetSlot->EquippedItem.ItemId = ResultItemId;
    TargetSlot->EquippedItem.Quantity = 1;
}
}

bool FInventoryCraftService::TryCraftByRecipeId(UInventoryComponent* InventoryComponent, FName RecipeId, EInventoryActionFailure* OutFailure)
{
    SetFailure_Craft(OutFailure, EInventoryActionFailure::None);

    if (!InventoryComponent || !InventoryComponent->CraftingRecipeTable || RecipeId.IsNone())
    {
        SetFailure_Craft(OutFailure, EInventoryActionFailure::InvalidRecipe);
        return false;
    }

    const FCraftingRecipeRow* RecipeRow = InventoryComponent->CraftingRecipeTable->FindRow<FCraftingRecipeRow>(RecipeId, TEXT("TryCraftByRecipeId"));
    if (!RecipeRow || !RecipeRow->ResultItemId.IsValid() || RecipeRow->ResultQuantity <= 0)
    {
        SetFailure_Craft(OutFailure, EInventoryActionFailure::InvalidRecipe);
        return false;
    }

    const UInventoryItemDataAsset* ResultItemData = ResolveItemData_Craft(RecipeRow->ResultItemId);
    const bool bIsEquipmentResult = IsEquipmentResult(ResultItemData);
    const EEquippableType ResultSlotType = bIsEquipmentResult ? ResultItemData->EquippableType : EEquippableType::None;

    for (const FCraftingIngredientRow& Ingredient : RecipeRow->Ingredients)
    {
        if (!Ingredient.ItemId.IsValid() || Ingredient.Quantity <= 0)
        {
            SetFailure_Craft(OutFailure, EInventoryActionFailure::InvalidRecipe);
            return false;
        }

        if (CountItemTotal(InventoryComponent, Ingredient.ItemId) < Ingredient.Quantity)
        {
            SetFailure_Craft(OutFailure, EInventoryActionFailure::MissingIngredient);
            return false;
        }
    }

    if (!bIsEquipmentResult && !CanStoreResultItem(InventoryComponent, RecipeRow->ResultItemId, RecipeRow->ResultQuantity))
    {
        SetFailure_Craft(OutFailure, EInventoryActionFailure::NoSpace);
        return false;
    }

    bool bUsedEquippedIngredient = false;
    EEquippableType LastConsumedEquippedSlotType = EEquippableType::None;
    for (const FCraftingIngredientRow& Ingredient : RecipeRow->Ingredients)
    {
        int32 Remaining = Ingredient.Quantity;
        for (FInventorySlotData& Slot : InventoryComponent->InventorySlots)
        {
            if (Remaining <= 0)
            {
                break;
            }

            if (!Slot.IsEmpty() && Slot.Item.ItemId == Ingredient.ItemId)
            {
                const int32 ConsumeCount = FMath::Min(Remaining, Slot.Item.Quantity);
                Slot.Item.Quantity -= ConsumeCount;
                Remaining -= ConsumeCount;

                if (Slot.Item.Quantity <= 0)
                {
                    Slot.Item = FInventoryItemHandle();
                }
            }
        }

        if (Remaining > 0)
        {
            ConsumeIngredientFromEquipment(InventoryComponent, Ingredient.ItemId, Remaining, bUsedEquippedIngredient, LastConsumedEquippedSlotType);
        }
    }

    int32 RemainingResult = RecipeRow->ResultQuantity;
    if (bIsEquipmentResult && RemainingResult > 0 && ShouldAutoEquipCraftResult(InventoryComponent, ResultSlotType, bUsedEquippedIngredient, LastConsumedEquippedSlotType))
    {
        EquipCraftResultOrFallback(InventoryComponent, ResultSlotType, RecipeRow->ResultItemId);
        RemainingResult -= 1;
    }

    RemainingResult = StoreItemToInventory(InventoryComponent, RecipeRow->ResultItemId, RemainingResult);

    if (RemainingResult > 0)
    {
        if (bIsEquipmentResult)
        {
            DropCraftedItem(InventoryComponent, RecipeRow->ResultItemId, RemainingResult);
            return true;
        }

        SetFailure_Craft(OutFailure, EInventoryActionFailure::NoSpace);
        return false;
    }

    return true;
}
