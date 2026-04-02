#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Core/InventoryTypes.h"
#include "GameplayEffectTypes.h"
#include "InventoryComponent.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryDropRequested, const FInventoryDropRequest&, DropRequest);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Inventory")
    int32 MaxInventorySlots = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    bool bUseEquipmentSlots = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    bool bGiveDefaultWeaponOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    FPrimaryAssetId DefaultWeaponItemId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
    TObjectPtr<UDataTable> CraftingRecipeTable = nullptr;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlotData> InventorySlots;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Equipment")
    FEquipmentSlotData WeaponSlot;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Equipment")
    FEquipmentSlotData TopArmorSlot;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Equipment")
    FEquipmentSlotData BottomArmorSlot;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryDropRequested OnInventoryDropRequested;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitializeInventorySlots();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryTransferItemAuto(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryTransferItemToSlot(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryDropAllFromSlot(int32 FromSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryDropSingleFromSlot(int32 FromSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool TryEquipItemFromSlot(int32 FromSlotIndex, EEquippableType TargetSlotType);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool TryUnequipTopArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool TryUnequipBottomArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool TryCraftItem(FName RecipeId);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool TryConsumeItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Init")
    bool EnsureDefaultEquipment();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerTryMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerTryTransferItemAuto(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerTryTransferItemToSlot(UInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerTryDropAllFromSlot(int32 FromSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerTryDropSingleFromSlot(int32 FromSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerTryEquipItemFromSlot(int32 FromSlotIndex, EEquippableType TargetSlotType);

    UFUNCTION(Server, Reliable)
    void ServerTryUnequipTopArmor();

    UFUNCTION(Server, Reliable)
    void ServerTryUnequipBottomArmor();

    UFUNCTION(Server, Reliable)
    void ServerTryCraftItem(FName RecipeId);

    UFUNCTION(Server, Reliable)
    void ServerTryConsumeItemAtSlot(int32 SlotIndex);

    FEquipmentSlotData* GetEquipmentSlotData(EEquippableType SlotType);
    FActiveGameplayEffectHandle* GetEquipmentEffectHandle(EEquippableType SlotType);
    bool SyncEquipEffectForSlot(EEquippableType SlotType);

    FActiveGameplayEffectHandle WeaponEquipEffectHandle;
    FActiveGameplayEffectHandle TopArmorEquipEffectHandle;
    FActiveGameplayEffectHandle BottomArmorEquipEffectHandle;
};
