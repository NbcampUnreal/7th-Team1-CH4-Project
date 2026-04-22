#include "Inventory/World/EDDroppedItemActor.h"

#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/AssetManager.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/EDAssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Net/UnrealNetwork.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveItemData_Drop(const FPrimaryAssetId& ItemId)
{
    if (!ItemId.IsValid())
    {
        return nullptr;
    }

    UEDAssetManager& AM = UEDAssetManager::Get(); 
    if (UEDInventoryItemDataAsset* CachedAsset = AM.GetPrimaryAsset<UEDInventoryItemDataAsset>(ItemId))
    {
        return CachedAsset;
    }
    return AM.LoadPrimaryAssetSync<UEDInventoryItemDataAsset>(ItemId);
}
}

AEDDroppedItemActor::AEDDroppedItemActor()
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;
    SetReplicateMovement(true);

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    ItemBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("ItemBillboard"));
    ItemBillboard->SetupAttachment(Root);
}

void AEDDroppedItemActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bAutoFaceToCamera)
    {
        UpdateFacingToLocalCamera();
    }
}

void AEDDroppedItemActor::InitializeDroppedItem(const FPrimaryAssetId& InItemId, int32 InQuantity)
{
    if (!HasAuthority() || !InItemId.IsValid() || InQuantity <= 0)
    {
        return;
    }

    DroppedItem.ItemId = InItemId;
    DroppedItem.Quantity = InQuantity;
    RefreshVisualData();
}

bool AEDDroppedItemActor::TryMergeDroppedItem(const FPrimaryAssetId& InItemId, int32 InQuantity)
{
    if (!HasAuthority() || !DroppedItem.IsValid() || !InItemId.IsValid() || InQuantity <= 0)
    {
        return false;
    }

    if (DroppedItem.ItemId != InItemId)
    {
        return false;
    }

    DroppedItem.Quantity += InQuantity;
    RefreshVisualData();
    return true;
}

FPrimaryAssetId AEDDroppedItemActor::GetItemId() const
{
    return DroppedItem.ItemId;
}

int32 AEDDroppedItemActor::GetQuantity() const
{
    return DroppedItem.Quantity;
}

FText AEDDroppedItemActor::GetItemDisplayName() const
{
    return CachedDisplayName;
}

EEDItemRarity AEDDroppedItemActor::GetItemRarity() const
{
    return CachedRarity;
}

UTexture2D* AEDDroppedItemActor::GetItemIconTexture() const
{
    return CachedIconTexture;
}

FLinearColor AEDDroppedItemActor::GetRarityColor() const
{
    return ResolveRarityColor(CachedRarity);
}

FText AEDDroppedItemActor::GetHoverDisplayText() const
{
    const FText NameText = CachedDisplayName.IsEmpty() ? FText::FromString(DroppedItem.ItemId.ToString()) : CachedDisplayName;
    if (DroppedItem.Quantity > 1)
    {
        return FText::Format(NSLOCTEXT("EDDroppedItem", "HoverWithQuantity", "{0} [x{1}]"), NameText, FText::AsNumber(DroppedItem.Quantity));
    }

    return NameText;
}

bool AEDDroppedItemActor::UpdateFacingToLocalCamera()
{
    if (!GetWorld() || GetNetMode() == NM_DedicatedServer)
    {
        return false;
    }

    USceneComponent* TargetComponent = Cast<USceneComponent>(CameraFacingTarget.GetComponent(this));
    if (!TargetComponent)
    {
        TargetComponent = ItemBillboard;
    }
    if (!TargetComponent)
    {
        return false;
    }

    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!CameraManager)
    {
        return false;
    }

    FVector ToCamera = CameraManager->GetCameraLocation() - TargetComponent->GetComponentLocation();
    if (bFaceYawOnly)
    {
        ToCamera.Z = 0.0f;
    }

    if (ToCamera.IsNearlyZero())
    {
        return false;
    }

    FRotator TargetRotation = ToCamera.Rotation();
    if (bFaceYawOnly)
    {
        TargetRotation.Pitch = 0.0f;
        TargetRotation.Roll = 0.0f;
    }

    TargetComponent->SetWorldRotation(TargetRotation);
    return true;
}

bool AEDDroppedItemActor::SetEffectColorParameter(FName ParameterName, FLinearColor InColor)
{
    UParticleSystemComponent* EffectComp = Cast<UParticleSystemComponent>(EffectComponent.GetComponent(this));
    if (!EffectComp || ParameterName.IsNone())
    {
        return false;
    }

    EffectComp->SetColorParameter(ParameterName, InColor);
    return true;
}

void AEDDroppedItemActor::OnRep_DroppedItem()
{
    RefreshVisualData();
}

void AEDDroppedItemActor::RefreshVisualData()
{
    CachedDisplayName = FText::GetEmpty();
    CachedRarity = EEDItemRarity::Normal;
    CachedIconTexture = nullptr;

    if (DroppedItem.IsValid())
    {
        const UEDInventoryItemDataAsset* ItemData = ResolveItemData_Drop(DroppedItem.ItemId);
        if (ItemData)
        {
            CachedDisplayName = ItemData->DisplayName.IsEmpty() ? FText::FromName(DroppedItem.ItemId.PrimaryAssetName) : ItemData->DisplayName;
            CachedRarity = ItemData->Rarity;
            CachedIconTexture = ItemData->IconTexture;
        }
        else
        {
            CachedDisplayName = FText::FromString(DroppedItem.ItemId.ToString());
        }
    }

    if (ItemBillboard)
    {
        ItemBillboard->SetSprite(CachedIconTexture);
    }

    if (bAutoApplyRarityColorToEffect)
    {
        SetEffectColorParameter(EffectColorParameterName, ResolveRarityColor(CachedRarity));
    }

    BP_OnDropVisualUpdated();
}

FLinearColor AEDDroppedItemActor::ResolveRarityColor(EEDItemRarity InRarity)
{
    switch (InRarity)
    {
    case EEDItemRarity::Rare:
        return FLinearColor(0.12f, 0.44f, 1.0f, 1.0f);
    case EEDItemRarity::Epic:
        return FLinearColor(0.60f, 0.20f, 0.90f, 1.0f);
    case EEDItemRarity::Legendary:
        return FLinearColor(1.0f, 0.85f, 0.10f, 1.0f);
    case EEDItemRarity::Unique:
        return FLinearColor(1.0f, 0.20f, 0.20f, 1.0f);
    case EEDItemRarity::Normal:
    default:
        return FLinearColor::White;
    }
}

void AEDDroppedItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEDDroppedItemActor, DroppedItem);
}
