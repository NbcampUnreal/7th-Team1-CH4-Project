#include "Tests/Inventory/EDTestBlueprintLibrary.h"

#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectIterator.h"

namespace
{
FString ResolveItemName(const FPrimaryAssetId& ItemId)
{
    if (!ItemId.IsValid())
    {
        return TEXT("None");
    }

    UAssetManager& AssetManager = UAssetManager::Get();
    if (UObject* AssetObject = AssetManager.GetPrimaryAssetObject(ItemId))
    {
        if (const UEDInventoryItemDataAsset* ItemData = Cast<UEDInventoryItemDataAsset>(AssetObject))
        {
            if (!ItemData->DisplayName.IsEmpty())
            {
                return ItemData->DisplayName.ToString();
            }
        }
    }

    return ItemId.PrimaryAssetName.ToString();
}

FString FormatItemLine(const FEDInventoryItemHandle& Item)
{
    return FString::Printf(TEXT("%s x%d"), *ResolveItemName(Item.ItemId), Item.Quantity);
}

void AppendEquipmentLine(TArray<FString>& OutLines, const TCHAR* SlotLabel, const FEDEquipmentSlotData& EquipmentSlot)
{
    if (!EquipmentSlot.EquippedItem.IsValid())
    {
        return;
    }

    OutLines.Add(FString::Printf(TEXT("%s: %s"), SlotLabel, *FormatItemLine(EquipmentSlot.EquippedItem)));
}

bool PassesAuthorityFilter(const UEDInventoryComponent* InventoryComponent, bool bOnlyAuthority)
{
    if (!bOnlyAuthority)
    {
        return true;
    }

    const AActor* OwnerActor = IsValid(InventoryComponent) ? InventoryComponent->GetOwner() : nullptr;
    return IsValid(OwnerActor) && OwnerActor->HasAuthority();
}

const TCHAR* ToRarityLabel(EEDItemRarity Rarity)
{
    switch (Rarity)
    {
    case EEDItemRarity::Normal:
        return TEXT("Normal");
    case EEDItemRarity::Rare:
        return TEXT("Rare");
    case EEDItemRarity::Epic:
        return TEXT("Epic");
    case EEDItemRarity::Legendary:
        return TEXT("Legendary");
    case EEDItemRarity::Unique:
        return TEXT("Unique");
    default:
        return TEXT("Unknown");
    }
}

FString FormatCraftableLine(const FEDCraftableRecipeEntry& Entry)
{
    const FString ItemName = Entry.ResultItemName.IsEmpty() ? Entry.ResultItemId.PrimaryAssetName.ToString() : Entry.ResultItemName.ToString();
    return FString::Printf(TEXT("%s -> %s (%s)"), *Entry.RowId.ToString(), *ItemName, ToRarityLabel(Entry.ResultRarity));
}

bool BuildInventoryBlock(const UEDInventoryComponent* InventoryComponent, bool bOnlyAuthority, FString& OutBlock)
{
    OutBlock.Empty();

    if (!IsValid(InventoryComponent) || !PassesAuthorityFilter(InventoryComponent, bOnlyAuthority))
    {
        return false;
    }

    const AActor* OwnerActor = InventoryComponent->GetOwner();
    if (!IsValid(OwnerActor))
    {
        return false;
    }

    TArray<FString> EquipmentLines;
    if (InventoryComponent->bUseEquipmentSlots)
    {
        AppendEquipmentLine(EquipmentLines, TEXT("weapon"), InventoryComponent->WeaponSlot);
        AppendEquipmentLine(EquipmentLines, TEXT("toparmor"), InventoryComponent->TopArmorSlot);
        AppendEquipmentLine(EquipmentLines, TEXT("bottomarmor"), InventoryComponent->BottomArmorSlot);
    }

    TArray<FString> InventoryLines;
    for (int32 SlotIndex = 0; SlotIndex < InventoryComponent->InventorySlots.Num(); ++SlotIndex)
    {
        const FEDInventorySlotData& SlotData = InventoryComponent->InventorySlots[SlotIndex];
        if (!SlotData.Item.IsValid())
        {
            continue;
        }

        InventoryLines.Add(FString::Printf(TEXT("slot%d: %s"), SlotIndex, *FormatItemLine(SlotData.Item)));
    }

    TArray<FString> CraftableLines;
    for (const FEDCraftableRecipeEntry& Craftable : InventoryComponent->CachedCraftableRecipes)
    {
        CraftableLines.Add(FormatCraftableLine(Craftable));
    }

    if (EquipmentLines.Num() == 0 && InventoryLines.Num() == 0 && CraftableLines.Num() == 0)
    {
        return false;
    }

    OutBlock = OwnerActor->GetName();

    if (EquipmentLines.Num() > 0)
    {
        OutBlock += TEXT(" - ");
        OutBlock += FString::Join(EquipmentLines, TEXT(" / "));
    }

    if (InventoryLines.Num() > 0)
    {
        OutBlock += TEXT("\nInventory:\n");
        OutBlock += FString::Join(InventoryLines, TEXT("\n"));
    }

    if (CraftableLines.Num() > 0)
    {
        OutBlock += TEXT("\nCraftable:\n");
        OutBlock += FString::Join(CraftableLines, TEXT("\n"));
    }

    return true;
}
}

AActor* UEDTestBlueprintLibrary::GetClosestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf)
{
    if (!IsValid(SourceActor) || !*ActorClass)
    {
        return nullptr;
    }

    UWorld* World = SourceActor->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Candidates);

    const FVector SourceLocation = SourceActor->GetActorLocation();
    AActor* ClosestActor = nullptr;
    float ClosestDistanceSq = TNumericLimits<float>::Max();

    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate))
        {
            continue;
        }

        if (!bIncludeSelf && Candidate == SourceActor)
        {
            continue;
        }

        const float DistanceSq = FVector::DistSquared(SourceLocation, Candidate->GetActorLocation());
        if (DistanceSq < ClosestDistanceSq)
        {
            ClosestDistanceSq = DistanceSq;
            ClosestActor = Candidate;
        }
    }

    return ClosestActor;
}

AActor* UEDTestBlueprintLibrary::GetFarthestActorOfClass(AActor* SourceActor, TSubclassOf<AActor> ActorClass, bool bIncludeSelf)
{
    if (!IsValid(SourceActor) || !*ActorClass)
    {
        return nullptr;
    }

    UWorld* World = SourceActor->GetWorld();
    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsOfClass(World, ActorClass, Candidates);

    const FVector SourceLocation = SourceActor->GetActorLocation();
    AActor* FarthestActor = nullptr;
    float FarthestDistanceSq = -1.0f;

    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate))
        {
            continue;
        }

        if (!bIncludeSelf && Candidate == SourceActor)
        {
            continue;
        }

        const float DistanceSq = FVector::DistSquared(SourceLocation, Candidate->GetActorLocation());
        if (DistanceSq > FarthestDistanceSq)
        {
            FarthestDistanceSq = DistanceSq;
            FarthestActor = Candidate;
        }
    }

    return FarthestActor;
}

FString UEDTestBlueprintLibrary::BuildAllInventoryDebugText(const UObject* WorldContextObject, bool bOnlyAuthority)
{
    TArray<UEDInventoryComponent*> InventoryComponents;
    GetAllWorldInventoryComponents(WorldContextObject, InventoryComponents, bOnlyAuthority);

    if (!IsValid(WorldContextObject))
    {
        return TEXT("InventoryDebug: Invalid WorldContextObject");
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return TEXT("InventoryDebug: World is null");
    }

    return BuildInventoryDebugTextFromArray(InventoryComponents, bOnlyAuthority);
}

FString UEDTestBlueprintLibrary::BuildInventoryDebugTextFromArray(const TArray<UEDInventoryComponent*>& InventoryComponents, bool bOnlyAuthority)
{
    TArray<FString> ActorBlocks;

    for (const UEDInventoryComponent* InventoryComponent : InventoryComponents)
    {
        FString Block;
        if (BuildInventoryBlock(InventoryComponent, bOnlyAuthority, Block))
        {
            ActorBlocks.Add(MoveTemp(Block));
        }
    }

    if (ActorBlocks.Num() == 0)
    {
        return bOnlyAuthority
            ? TEXT("InventoryDebug: No non-empty authoritative inventory/equipment found.")
            : TEXT("InventoryDebug: No non-empty inventory/equipment found.");
    }

    ActorBlocks.Sort();
    return FString::Join(ActorBlocks, TEXT("\n\n"));
}

void UEDTestBlueprintLibrary::GetAllWorldInventoryComponents(const UObject* WorldContextObject, TArray<UEDInventoryComponent*>& OutInventoryComponents, bool bOnlyAuthority)
{
    OutInventoryComponents.Reset();

    if (!IsValid(WorldContextObject))
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return;
    }

    for (TObjectIterator<UEDInventoryComponent> It; It; ++It)
    {
        UEDInventoryComponent* InventoryComponent = *It;
        if (!IsValid(InventoryComponent) || InventoryComponent->GetWorld() != World)
        {
            continue;
        }

        if (!PassesAuthorityFilter(InventoryComponent, bOnlyAuthority))
        {
            continue;
        }

        OutInventoryComponents.Add(InventoryComponent);
    }
}

void UEDTestBlueprintLibrary::PrintAllInventoryDebugText(const UObject* WorldContextObject, float Duration, bool bOnlyAuthority)
{
    const FString DebugText = BuildAllInventoryDebugText(WorldContextObject, bOnlyAuthority);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(777002, Duration, FColor::Cyan, DebugText);
    }
}
