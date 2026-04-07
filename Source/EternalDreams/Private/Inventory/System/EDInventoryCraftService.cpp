#include "Inventory/System/EDInventoryCraftService.h"

#include "Engine/AssetManager.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Item/Data/EDItemDataRows.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveItemData_Craft(const FPrimaryAssetId& ItemId)
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

    return Cast<UEDInventoryItemDataAsset>(ItemObject);
}

int32 GetItemMaxStack_Craft(const FPrimaryAssetId& ItemId)
{
    const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Craft(ItemId);
    return ItemData ? FMath::Max(1, ItemData->MaxStack) : 1;
}

void SetFailure_Craft(EEDInventoryActionFailure* OutFailure, EEDInventoryActionFailure Failure)
{
    if (OutFailure)
    {
        *OutFailure = Failure;
    }
}

int32 CountItemInInventory(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    int32 Count = 0;
    for (const FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
    {
        if (!Slot.IsEmpty() && Slot.Item.ItemId == ItemId)
        {
            Count += Slot.Item.Quantity;
        }
    }

    return Count;
}

bool CanStoreResultItem(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity)
{
    const int32 MaxStack = GetItemMaxStack_Craft(ItemId);
    int32 Capacity = 0;

    for (const FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
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

bool IsEquipmentResult(const UEDInventoryItemDataAsset* ItemData)
{
    return ItemData && ItemData->ItemType == EEDInventoryItemType::Equippable && ItemData->EquippableType != EEDEquippableType::None;
}

FEDEquipmentSlotData* GetEquipmentSlot_Craft(UEDInventoryComponent* InventoryComponent, EEDEquippableType SlotType)
{
    if (!InventoryComponent)
    {
        return nullptr;
    }

    switch (SlotType)
    {
    case EEDEquippableType::Weapon:
        return &InventoryComponent->WeaponSlot;
    case EEDEquippableType::TopArmor:
        return &InventoryComponent->TopArmorSlot;
    case EEDEquippableType::BottomArmor:
        return &InventoryComponent->BottomArmorSlot;
    default:
        return nullptr;
    }
}

int32 CountItemInEquipment(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    int32 Count = 0;

    const TArray<const FEDEquipmentSlotData*> EquipmentSlots =
    {
        &InventoryComponent->WeaponSlot,
        &InventoryComponent->TopArmorSlot,
        &InventoryComponent->BottomArmorSlot
    };

    for (const FEDEquipmentSlotData* Slot : EquipmentSlots)
    {
        if (Slot && Slot->EquippedItem.IsValid() && Slot->EquippedItem.ItemId == ItemId)
        {
            Count += Slot->EquippedItem.Quantity;
        }
    }

    return Count;
}

int32 CountItemTotal(const UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId)
{
    return CountItemInInventory(InventoryComponent, ItemId) + CountItemInEquipment(InventoryComponent, ItemId);
}

int32 StoreItemToInventory(UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity)
{
    if (!InventoryComponent || !ItemId.IsValid() || Quantity <= 0)
    {
        return Quantity;
    }

    const int32 MaxStack = GetItemMaxStack_Craft(ItemId);
    int32 Remaining = Quantity;

    for (FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
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

    for (FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
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

void DropCraftedItem(UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32 Quantity)
{
    if (!InventoryComponent || !ItemId.IsValid() || Quantity <= 0)
    {
        return;
    }

    FEDInventoryDropRequest DropRequest;
    DropRequest.Item.ItemId = ItemId;
    DropRequest.Item.Quantity = Quantity;
    DropRequest.SourceOwner = InventoryComponent->GetOwner();
    DropRequest.Reason = EEDInventoryDropReason::CraftSwap;
    InventoryComponent->OnInventoryDropRequested.Broadcast(DropRequest);
}

bool ConsumeIngredientFromEquipment(UEDInventoryComponent* InventoryComponent, const FPrimaryAssetId& ItemId, int32& InOutRemaining, bool& bOutUsedEquippedIngredient, EEDEquippableType& OutLastConsumedSlotType)
{
    if (!InventoryComponent || InOutRemaining <= 0)
    {
        return true;
    }

    const TArray<EEDEquippableType> ConsumeOrder =
    {
        EEDEquippableType::Weapon,
        EEDEquippableType::TopArmor,
        EEDEquippableType::BottomArmor
    };

    for (EEDEquippableType SlotType : ConsumeOrder)
    {
        if (InOutRemaining <= 0)
        {
            break;
        }

        FEDEquipmentSlotData* Slot = GetEquipmentSlot_Craft(InventoryComponent, SlotType);
        if (Slot && Slot->EquippedItem.IsValid() && Slot->EquippedItem.ItemId == ItemId)
        {
            const int32 ConsumeCount = FMath::Min(InOutRemaining, Slot->EquippedItem.Quantity);
            Slot->EquippedItem.Quantity -= ConsumeCount;
            InOutRemaining -= ConsumeCount;
            bOutUsedEquippedIngredient = true;
            OutLastConsumedSlotType = SlotType;

            if (Slot->EquippedItem.Quantity <= 0)
            {
                Slot->EquippedItem = FEDInventoryItemHandle();
            }
        }
    }

    return InOutRemaining <= 0;
}

bool ShouldAutoEquipCraftResult(UEDInventoryComponent* InventoryComponent, EEDEquippableType ResultSlotType, bool bUsedEquippedIngredient, EEDEquippableType LastConsumedSlotType)
{
    if (!InventoryComponent || !InventoryComponent->bUseEquipmentSlots)
    {
        return false;
    }

    if (ResultSlotType == EEDEquippableType::Weapon)
    {
        return true;
    }

    FEDEquipmentSlotData* ResultSlot = GetEquipmentSlot_Craft(InventoryComponent, ResultSlotType);
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

void EquipCraftResultOrFallback(UEDInventoryComponent* InventoryComponent, EEDEquippableType SlotType, const FPrimaryAssetId& ResultItemId)
{
    FEDEquipmentSlotData* TargetSlot = GetEquipmentSlot_Craft(InventoryComponent, SlotType);
    if (!InventoryComponent || !TargetSlot || !ResultItemId.IsValid())
    {
        return;
    }

    if (TargetSlot->EquippedItem.IsValid())
    {
        const FEDInventoryItemHandle PreviousEquippedItem = TargetSlot->EquippedItem;
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

bool FEDInventoryCraftService::TryCraftByRecipeId(UEDInventoryComponent* InventoryComponent, FName RecipeId, EEDInventoryActionFailure* OutFailure)
{
    SetFailure_Craft(OutFailure, EEDInventoryActionFailure::None);

    if (!InventoryComponent || !InventoryComponent->CraftingRecipeTable || RecipeId.IsNone())
    {
        SetFailure_Craft(OutFailure, EEDInventoryActionFailure::InvalidRecipe);
        return false;
    }

    const FEDCraftingRecipeRow* RecipeRow = InventoryComponent->CraftingRecipeTable->FindRow<FEDCraftingRecipeRow>(RecipeId, TEXT("TryCraftByRecipeId"));
    if (!RecipeRow || !RecipeRow->ResultItemId.IsValid() || RecipeRow->ResultQuantity <= 0)
    {
        SetFailure_Craft(OutFailure, EEDInventoryActionFailure::InvalidRecipe);
        return false;
    }

    const UEDInventoryItemDataAsset* ResultItemData = ResolveItemData_Craft(RecipeRow->ResultItemId);
    const bool bIsEquipmentResult = IsEquipmentResult(ResultItemData);
    const EEDEquippableType ResultSlotType = bIsEquipmentResult ? ResultItemData->EquippableType : EEDEquippableType::None;

    for (const FEDCraftingIngredientRow& Ingredient : RecipeRow->Ingredients)
    {
        if (!Ingredient.ItemId.IsValid() || Ingredient.Quantity <= 0)
        {
            SetFailure_Craft(OutFailure, EEDInventoryActionFailure::InvalidRecipe);
            return false;
        }

        if (CountItemTotal(InventoryComponent, Ingredient.ItemId) < Ingredient.Quantity)
        {
            SetFailure_Craft(OutFailure, EEDInventoryActionFailure::MissingIngredient);
            return false;
        }
    }

    if (!bIsEquipmentResult && !CanStoreResultItem(InventoryComponent, RecipeRow->ResultItemId, RecipeRow->ResultQuantity))
    {
        SetFailure_Craft(OutFailure, EEDInventoryActionFailure::NoSpace);
        return false;
    }

    bool bUsedEquippedIngredient = false;
    EEDEquippableType LastConsumedEquippedSlotType = EEDEquippableType::None;
    for (const FEDCraftingIngredientRow& Ingredient : RecipeRow->Ingredients)
    {
        int32 Remaining = Ingredient.Quantity;
        for (FEDInventorySlotData& Slot : InventoryComponent->InventorySlots)
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
                    Slot.Item = FEDInventoryItemHandle();
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

        SetFailure_Craft(OutFailure, EEDInventoryActionFailure::NoSpace);
        return false;
    }

    return true;
}
