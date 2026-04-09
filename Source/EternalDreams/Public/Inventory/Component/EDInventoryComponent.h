#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "GameplayEffectTypes.h"
#include "EDInventoryComponent.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEDInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEDInventoryDropRequested, const FEDInventoryDropRequest&, DropRequest);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UEDInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEDInventoryComponent();

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot")
    TObjectPtr<UDataTable> RandomLootTable = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot", meta = (ClampMin = "0"))
    int32 RandomLootRollCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot")
    EEDInventoryLootSpawnMode RandomLootSpawnMode = EEDInventoryLootSpawnMode::Static;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot", meta = (ClampMin = "0"))
    int32 RandomLootMaxTotalQuantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot")
    int32 RandomLootMinIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot")
    int32 RandomLootMaxIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot")
    int32 RandomLootSeed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Loot")
    bool bAutoInitializeRandomLootOnBeginPlay = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Loot")
    bool bRandomLootInitialized = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_InventorySlots, Category="Inventory")
    TArray<FEDInventorySlotData> InventorySlots;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponSlot, Category = "Inventory|Equipment")
    FEDEquipmentSlotData WeaponSlot;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_TopArmorSlot, Category = "Inventory|Equipment")
    FEDEquipmentSlotData TopArmorSlot;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_BottomArmorSlot, Category = "Inventory|Equipment")
    FEDEquipmentSlotData BottomArmorSlot;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Craft")
    bool bAutoRefreshCraftableRecipesCache = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Craft")
    EEDCraftableRecipeSortOption CachedCraftableRecipesSortOption = EEDCraftableRecipeSortOption::ByRowId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Craft")
    bool bCachedCraftableRecipesDescending = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Craft")
    TArray<FEDCraftableRecipeEntry> CachedCraftableRecipes;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnEDInventoryChanged OnInventoryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnEDInventoryDropRequested OnInventoryDropRequested;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitializeInventorySlots();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryTransferItemAuto(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryTransferItemAutoDetailed(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryTransferItemToSlot(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryTransferItemToSlotDetailed(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryDropAllFromSlot(int32 FromSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryDropSingleFromSlot(int32 FromSlotIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool TryEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool TryUnequipTopArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool TryUnequipBottomArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItemAutoDetailed(FPrimaryAssetId ItemId, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryAddItemToSlotDetailed(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
    bool TryInitializeRandomLoot();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
    bool TryInitializeRandomLootDetailed(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool TryCraftItem(FName RecipeId);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool TryCraftItemDetailed(FName RecipeId, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    void GetCraftableRecipes(TArray<FEDCraftableRecipeEntry>& OutRecipes, EEDCraftableRecipeSortOption SortOption = EEDCraftableRecipeSortOption::ByRowId, bool bDescending = false) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool CanCraftRecipeByRowId(FName RecipeRowId) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    void RefreshCraftableRecipesCache();

    UFUNCTION(BlueprintPure, Category = "Inventory|Craft")
    void GetCachedCraftableRecipes(TArray<FEDCraftableRecipeEntry>& OutRecipes) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    void SetCraftableRecipeCacheSort(EEDCraftableRecipeSortOption SortOption, bool bDescending);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool TryCraftFirstCachedRecipe();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool TryConsumeItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool TryConsumeItemAtSlotDetailed(int32 SlotIndex, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Init")
    bool EnsureDefaultEquipment();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerTryMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerTryTransferItemAuto(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerTryTransferItemToSlot(UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerTryDropAllFromSlot(int32 FromSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerTryDropSingleFromSlot(int32 FromSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerTryAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerTryAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerTryInitializeRandomLoot(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed);

    UFUNCTION(Server, Reliable)
    void ServerTryEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType);

    UFUNCTION(Server, Reliable)
    void ServerTryUnequipTopArmor();

    UFUNCTION(Server, Reliable)
    void ServerTryUnequipBottomArmor();

    UFUNCTION(Server, Reliable)
    void ServerTryCraftItem(FName RecipeId);

    UFUNCTION(Server, Reliable)
    void ServerTryConsumeItemAtSlot(int32 SlotIndex);

    UFUNCTION()
    void OnRep_InventorySlots();

    UFUNCTION()
    void OnRep_WeaponSlot();

    UFUNCTION()
    void OnRep_TopArmorSlot();

    UFUNCTION()
    void OnRep_BottomArmorSlot();

    UFUNCTION()
    void HandleInventoryChangedInternal();

    FEDEquipmentSlotData* GetEquipmentSlotData(EEDEquippableType SlotType);
    FActiveGameplayEffectHandle* GetEquipmentEffectHandle(EEDEquippableType SlotType);
    bool SyncEquipEffectForSlot(EEDEquippableType SlotType);

    FActiveGameplayEffectHandle WeaponEquipEffectHandle;
    FActiveGameplayEffectHandle TopArmorEquipEffectHandle;
    FActiveGameplayEffectHandle BottomArmorEquipEffectHandle;
};
