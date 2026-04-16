#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "GameplayEffectTypes.h"
#include "TimerManager.h"
#include "EDInventoryComponent.generated.h"

class UDataTable;
class AActor;
class AEDDroppedItemActor;

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
    TArray<TObjectPtr<UDataTable>> CraftingRecipeTables;

    UFUNCTION(BlueprintPure, Category = "Inventory|Craft")
    void GetAllCraftingRecipeTables(TArray<UDataTable*>& OutTables) const;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Distribution")
    bool bAutoDistributeInventoryOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Distribution")
    EEDInventorySplitMode DistributionMode = EEDInventorySplitMode::SplitByCount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Distribution", meta = (EditCondition = "DistributionMode == EEDInventorySplitMode::SplitByRarity", EditConditionHides))
    EEDInventoryRaritySecondarySplitMode DistributionRaritySecondaryMode = EEDInventoryRaritySecondarySplitMode::ByCount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Distribution")
    TArray<TObjectPtr<AActor>> DistributionTargetActors;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Distribution", meta = (ClampMin = "0.0"))
    float DistributionTargetWaitTimeout = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Distribution", meta = (ClampMin = "0.01"))
    float DistributionRetryInterval = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Distribution")
    int32 DistributionSeed = 0;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Distribution")
    bool bDistributionCompleted = false;

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


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    bool bSpawnDroppedItemActor = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    TSubclassOf<AEDDroppedItemActor> DroppedItemActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    FVector DroppedItemSpawnOffset = FVector(100.0f, 0.0f, 30.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Drop", meta = (ClampMin = "0.0"))
    float DroppedItemMergeRadius = 150.0f;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RequestInitializeInventorySlots();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool PredicateMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestMoveItemBetweenSlots(int32 FromSlotIndex, int32 ToSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool PredicateTransferItemAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool RequestTransferItemAuto(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool PredicateTransferItemToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Network")
    bool RequestTransferItemToSlot(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity);

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateTransferItemAuto + RequestTransferItemAuto"), Category = "Inventory|Network")
    bool RequestTransferItemAutoDetailed(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateTransferItemToSlot + RequestTransferItemToSlot"), Category = "Inventory|Network")
    bool RequestTransferItemToSlotDetailed(UEDInventoryComponent* FromInventory, UEDInventoryComponent* ToInventory, int32 FromSlotIndex, int32 ToSlotIndex, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool PredicateDropAllFromSlot(int32 FromSlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestDropAllFromSlot(int32 FromSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool PredicateDropSingleFromSlot(int32 FromSlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestDropSingleFromSlot(int32 FromSlotIndex);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool PredicateDropCountFromSlot(int32 FromSlotIndex, int32 DropCount, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestDropCountFromSlot(int32 FromSlotIndex, int32 DropCount);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Drop")
    bool PredicatePickupDroppedItem(AEDDroppedItemActor* DroppedItemActor, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Drop")
    bool RequestPickupDroppedItem(AEDDroppedItemActor* DroppedItemActor);

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicatePickupDroppedItem + RequestPickupDroppedItem"), Category = "Inventory|Drop")
    bool RequestPickupDroppedItemDetailed(AEDDroppedItemActor* DroppedItemActor, EEDInventoryActionFailure& OutFailure);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool PredicateEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool RequestEquipItemFromSlot(int32 FromSlotIndex, EEDEquippableType TargetSlotType);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool PredicateUnequipTopArmor(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool RequestUnequipTopArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool PredicateUnequipBottomArmor(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Equipment")
    bool RequestUnequipBottomArmor();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool PredicateAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity);

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateAddItemAuto + RequestAddItemAuto"), Category = "Inventory")
    bool RequestAddItemAutoDetailed(FPrimaryAssetId ItemId, int32 Quantity, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool PredicateAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RequestAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex);

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateAddItemToSlot + RequestAddItemToSlot"), Category = "Inventory")
    bool RequestAddItemToSlotDetailed(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
    bool PredicateInitializeRandomLoot(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Loot")
    bool RequestInitializeRandomLoot();

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateInitializeRandomLoot + RequestInitializeRandomLoot"), Category = "Inventory|Loot")
    bool RequestInitializeRandomLootDetailed(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Distribution")
    bool PredicateDistributeInventoryToTargets(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Distribution")
    bool RequestDistributeInventoryToTargets();

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateDistributeInventoryToTargets + RequestDistributeInventoryToTargets"), Category = "Inventory|Distribution")
    bool RequestDistributeInventoryToTargetsDetailed(EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool PredicateCraftItem(FName RecipeId, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Craft")
    bool RequestCraftItem(FName RecipeId);

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateCraftItem + RequestCraftItem"), Category = "Inventory|Craft")
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
    bool PredicateConsumeItemAtSlot(int32 SlotIndex, EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
    bool RequestConsumeItemAtSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction, DeprecationMessage = "Use PredicateConsumeItemAtSlot + RequestConsumeItemAtSlot"), Category = "Inventory|Consumable")
    bool RequestConsumeItemAtSlotDetailed(int32 SlotIndex, EEDInventoryActionFailure& OutFailure);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Init")
    bool PredicateEnsureDefaultEquipment(EEDInventoryActionFailure& OutFailure, bool bAutoRequestIfValid = false);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Init")
    bool RequestEnsureDefaultEquipment();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



    UPROPERTY(EditAnywhere, meta = (DeprecatedFunction), BlueprintReadOnly, Category = "Inventory|Debug")
    bool bGiveDebugItemsOnBeginPlay = false;

    UPROPERTY(EditAnywhere, meta = (DeprecatedFunction), BlueprintReadOnly, Category = "Inventory|Debug")
    FPrimaryAssetId DebugConsumableItemId;

    UPROPERTY(EditAnywhere, meta = (DeprecatedFunction), BlueprintReadOnly, Category = "Inventory|Debug")
    FPrimaryAssetId DebugMaterialItemId;

    UPROPERTY(EditAnywhere, meta = (DeprecatedFunction), BlueprintReadOnly, Category = "Inventory|Debug")
    FPrimaryAssetId DebugEquipItemId;
    // ---

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
    void ServerRequestDropCountFromSlot(int32 FromSlotIndex, int32 DropCount);

    UFUNCTION(Server, Reliable)
    void ServerRequestPickupDroppedItem(AEDDroppedItemActor* DroppedItemActor);

    UFUNCTION(Server, Reliable)
    void ServerRequestAddItemAuto(FPrimaryAssetId ItemId, int32 Quantity);

    UFUNCTION(Server, Reliable)
    void ServerRequestAddItemToSlot(FPrimaryAssetId ItemId, int32 Quantity, int32 SlotIndex);

    UFUNCTION(Server, Reliable)
    void ServerRequestInitializeRandomLoot(int32 RollCount, int32 MinLootIndex, int32 MaxLootIndex, int32 Seed);

    UFUNCTION(Server, Reliable)
    void ServerRequestDistributeInventoryToTargets();

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

    UFUNCTION()
    void HandleDropRequestSpawnWorldItem(const FEDInventoryDropRequest& DropRequest);

    bool TrySpawnOrMergeDroppedItem(const FEDInventoryItemHandle& ItemHandle, const FVector& SpawnOrigin);


    void TryStartDeferredDistribution();
    void ProcessDeferredDistribution();
    bool IsReadyForDistribution() const;
    void GatherDistributionTargets(TArray<UEDInventoryComponent*>& OutReadyTargets, bool& bOutHasPendingTargets) const;
    bool ExecuteDistribution(const TArray<UEDInventoryComponent*>& ReadyTargets, EEDInventoryActionFailure& OutFailure);
    bool ExecuteDistributionByCount(const TArray<UEDInventoryComponent*>& ReadyTargets, FRandomStream& RandomStream);
    bool ExecuteDistributionByType(const TArray<UEDInventoryComponent*>& ReadyTargets, FRandomStream& RandomStream);
    bool ExecuteDistributionByRarity(const TArray<UEDInventoryComponent*>& ReadyTargets, FRandomStream& RandomStream);
    static bool IsInventoryReadyForDistribution(const UEDInventoryComponent* InventoryComponent);

    FEDEquipmentSlotData* GetEquipmentSlotData(EEDEquippableType SlotType);
    FActiveGameplayEffectHandle* GetEquipmentEffectHandle(EEDEquippableType SlotType);
    bool SyncEquipEffectForSlot(EEDEquippableType SlotType);
    FGameplayTagContainer* GetAppliedEquipTagsCache(EEDEquippableType SlotType);
    bool SyncEquipTagsForSlot(EEDEquippableType SlotType);

    FActiveGameplayEffectHandle WeaponEquipEffectHandle;
    FActiveGameplayEffectHandle TopArmorEquipEffectHandle;
    FActiveGameplayEffectHandle BottomArmorEquipEffectHandle;

    FGameplayTagContainer WeaponAppliedEquipTags;
    FGameplayTagContainer TopArmorAppliedEquipTags;
    FGameplayTagContainer BottomArmorAppliedEquipTags;

    FTimerHandle DeferredDistributionTimerHandle;
    float DeferredDistributionStartTime = 0.0f;
    
};
