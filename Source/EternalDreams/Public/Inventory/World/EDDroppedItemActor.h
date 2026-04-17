#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDDroppedItemActor.generated.h"

class UBillboardComponent;
class USceneComponent;
class UParticleSystemComponent;
class UTexture2D;

UCLASS()
class ETERNALDREAMS_API AEDDroppedItemActor : public AActor
{
    GENERATED_BODY()

public:
    AEDDroppedItemActor();

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Drop")
    void InitializeDroppedItem(const FPrimaryAssetId& InItemId, int32 InQuantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory|Drop")
    bool TryMergeDroppedItem(const FPrimaryAssetId& InItemId, int32 InQuantity);

    UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
    FPrimaryAssetId GetItemId() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
    int32 GetQuantity() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
    FText GetItemDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
    EEDItemRarity GetItemRarity() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
    UTexture2D* GetItemIconTexture() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
    FLinearColor GetRarityColor() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
    FText GetHoverDisplayText() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Drop|View")
    bool UpdateFacingToLocalCamera();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Drop|Effect")
    bool SetEffectColorParameter(FName ParameterName, FLinearColor InColor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Drop")
    void BP_OnDropVisualUpdated();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    TObjectPtr<UBillboardComponent> ItemBillboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop|View")
    bool bAutoFaceToCamera = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop|View")
    bool bFaceYawOnly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop|View")
    FComponentReference CameraFacingTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop|Effect")
    FComponentReference EffectComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop|Effect")
    bool bAutoApplyRarityColorToEffect = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Drop|Effect")
    FName EffectColorParameterName = TEXT("Color");

    UPROPERTY(ReplicatedUsing = OnRep_DroppedItem, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    FEDInventoryItemHandle DroppedItem;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Drop")
    FText CachedDisplayName;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Drop")
    EEDItemRarity CachedRarity = EEDItemRarity::Normal;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Inventory|Drop")
    TObjectPtr<UTexture2D> CachedIconTexture = nullptr;

    UFUNCTION()
    void OnRep_DroppedItem();

    void RefreshVisualData();
    static FLinearColor ResolveRarityColor(EEDItemRarity InRarity);
};
