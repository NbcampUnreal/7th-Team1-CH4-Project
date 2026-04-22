// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftableRecipeBarWidget.h"

#include "Characters/Player/EDPlayerController.h"
#include "Components/PanelWidget.h"
#include "Core/EDAssetManager.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/HUD/EDCraftableRecipeSlotWidget.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveCraftableItemData(const FPrimaryAssetId& ItemId)
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

void UEDCraftableRecipeBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeInventoryComponent();
	BindInventoryChanged();
	BindCraftInputTriggered();
	RefreshCraftableRecipeBar();
}

void UEDCraftableRecipeBarWidget::NativeDestruct()
{
	UnbindCraftInputTriggered();
	UnbindInventoryChanged();

	Super::NativeDestruct();
}

void UEDCraftableRecipeBarWidget::SetInventoryComponent(UEDInventoryComponent* InInventoryComponent)
{
	if (InventoryComponent == InInventoryComponent)
	{
		RefreshCraftableRecipeBar();
		return;
	}

	UnbindInventoryChanged();
	InventoryComponent = InInventoryComponent;
	BindInventoryChanged();
	RefreshCraftableRecipeBar();
}

void UEDCraftableRecipeBarWidget::RefreshCraftableRecipeBar()
{
	if (!CraftableRecipeBarContainer)
	{
		return;
	}

	CraftableRecipeBarContainer->ClearChildren();
	CraftableRecipeSlotWidgets.Reset();

	if (!InventoryComponent || !CraftableRecipeSlotWidgetClass)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	InventoryComponent->RefreshCraftableRecipesCache();

	TArray<FEDCraftableRecipeEntry> CraftableRecipeEntries;
	InventoryComponent->GetCachedCraftableRecipes(CraftableRecipeEntries);
	if (CraftableRecipeEntries.Num() <= 0)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);

	for (int32 RecipeIndex = 0; RecipeIndex < CraftableRecipeEntries.Num(); ++RecipeIndex)
	{
		const FEDCraftableRecipeEntry& RecipeEntry = CraftableRecipeEntries[RecipeIndex];

		UTexture2D* ResultIconTexture = nullptr;
		ResolveRecipeDisplayData(RecipeEntry, ResultIconTexture);

		UEDCraftableRecipeSlotWidget* SlotWidget = CreateWidget<UEDCraftableRecipeSlotWidget>(this, CraftableRecipeSlotWidgetClass);
		if (!SlotWidget)
		{
			continue;
		}

		FEDCraftableRecipeSlotDisplayData DisplayData;
		DisplayData.IconTexture = ResultIconTexture;
		DisplayData.Rarity = RecipeEntry.ResultRarity;
		DisplayData.bIsCurrentCraftTarget = (RecipeIndex == 0);

		SlotWidget->SetSlotDisplayData(DisplayData);
		CraftableRecipeBarContainer->AddChild(SlotWidget);
		CraftableRecipeSlotWidgets.Add(SlotWidget);
	}
}

void UEDCraftableRecipeBarWidget::InitializeInventoryComponent()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return;
	}

	InventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(OwningPawn);
}

void UEDCraftableRecipeBarWidget::BindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UEDCraftableRecipeBarWidget::HandleInventoryChanged);
}

void UEDCraftableRecipeBarWidget::UnbindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UEDCraftableRecipeBarWidget::HandleInventoryChanged);
}

void UEDCraftableRecipeBarWidget::BindCraftInputTriggered()
{
	AEDPlayerController* PlayerController = GetOwningPlayer<AEDPlayerController>();
	if (!PlayerController)
	{
		return;
	}

	BoundPlayerController = PlayerController;
	BoundPlayerController->GetOnCraftInputTriggered().RemoveAll(this);
	BoundPlayerController->GetOnCraftInputTriggered().AddUObject(this, &UEDCraftableRecipeBarWidget::HandleCraftInputTriggered);
}

void UEDCraftableRecipeBarWidget::UnbindCraftInputTriggered()
{
	if (!BoundPlayerController)
	{
		return;
	}

	BoundPlayerController->GetOnCraftInputTriggered().RemoveAll(this);
	BoundPlayerController = nullptr;
}

bool UEDCraftableRecipeBarWidget::ResolveRecipeDisplayData(const FEDCraftableRecipeEntry& InRecipeData, UTexture2D*& OutIconTexture) const
{
	OutIconTexture = nullptr;

	const UEDInventoryItemDataAsset* ItemData = ResolveCraftableItemData(InRecipeData.ResultItemId);
	if (!ItemData)
	{
		return false;
	}

	OutIconTexture = ItemData->IconTexture;
	return true;
}

void UEDCraftableRecipeBarWidget::HandleInventoryChanged()
{
	RefreshCraftableRecipeBar();
}

void UEDCraftableRecipeBarWidget::HandleCraftInputTriggered()
{
	RefreshCraftableRecipeBar();
}
