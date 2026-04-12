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
    bool bAutoInitializeLootOnBeginPlay = false;

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
    void RequestInitializeInventorySlots();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool RequestTransferItemAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool RequestTransferItemAutoDetailed(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool RequestTransferItemToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool RequestTransferItemToSlotDetailed(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestDropAllFromSlot(int32 FromSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestDropSingleFromSlot(int32 FromSlotIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool RequestEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool RequestUnequipTopArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool RequestUnequipBottomArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestAddItemAutoDetailed(FPrimaryAssetId ItemId, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestAddItemToSlotDetailed(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
    bool RequestInitializeRandomLoot();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
    bool RequestInitializeRandomLootDetailed(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool RequestCraftItem(FName RecipeId);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool RequestCraftItemDetailed(FName RecipeId, EEDInventoryActionFailure& OutFailure);

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
    bool RequestCraftFirstCachedRecipe();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool RequestConsumeItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool RequestConsumeItemAtSlotDetailed(int32 SlotIndex, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Init")
    bool RequestEnsureDefaultEquipment();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;

    UFUNCTION(Server, Reliable)
    void ServerRequestInitializeInventorySlots();

    UFUNCTION(Server, Reliable)
    void ServerRequestMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerRequestTransferItemAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerRequestTransferItemToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerRequestDropAllFromSlot(int32 FromSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerRequestDropSingleFromSlot(int32 FromSlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerRequestAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerRequestAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerRequestInitializeRandomLoot(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed);

    UFUNCTION(Server, Reliable)
    void ServerRequestEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType);

    UFUNCTION(Server, Reliable)
    void ServerRequestUnequipTopArmor();

    UFUNCTION(Server, Reliable)
    void ServerRequestUnequipBottomArmor();

    UFUNCTION(Server, Reliable)
    void ServerRequestCraftItem(FName RecipeId);

    UFUNCTION(Server, Reliable)
    void ServerRequestConsumeItemAtSlot(int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerRequestEnsureDefaultEquipment();

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
    
    // 테스트용 아이템을 BeginPlay 시 지급할지 여부
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Debug")
    bool bGiveDebugItemsOnBeginPlay = false;

    // 테스트용 소비 아이템
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Debug")
    FPrimaryAssetId DebugConsumableItemId;
    
    // 테스트용 재료 아이템
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Debug")
    FPrimaryAssetId DebugMaterialItemId;

    // 테스트용 장비 아이템
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Debug")
    FPrimaryAssetId DebugEquipItemId;
};
