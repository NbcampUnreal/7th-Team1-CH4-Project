// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDItemCraftingWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/HUD/EDCraftIngredientEntryWidget.h"
#include "UI/HUD/EDCraftRecipeEntryWidget.h"
#include "UI/HUD/EDCraftTreeNodeWidget.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveCraftTreeItemData(const FPrimaryAssetId& ItemId)
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
}

void UEDItemCraftingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeInventoryComponent();
	BindInventoryChanged();
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::NativeDestruct()
{
	UnbindInventoryChanged();

	Super::NativeDestruct();
}

void UEDItemCraftingWidget::SetInventoryComponent(UEDInventoryComponent* InInventoryComponent)
{
	if (InventoryComponent == InInventoryComponent)
	{
		RefreshCraftRecipes();
		return;
	}

	UnbindInventoryChanged();
	InventoryComponent = InInventoryComponent;
	BindInventoryChanged();
	RefreshCraftRecipes();
}

bool UEDItemCraftingWidget::RequestCraftSelectedRecipe()
{
	if (!InventoryComponent)
	{
		ShowCraftFailure(EEDInventoryActionFailure::InvalidInventory);
		return false;
	}

	if (SelectedRecipeRowId.IsNone())
	{
		ShowCraftFailure(EEDInventoryActionFailure::InvalidRecipe);
		return false;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSuccess = InventoryComponent->RequestCraftItemDetailed(SelectedRecipeRowId, Failure);
	if (!bSuccess)
	{
		ShowCraftFailure(Failure);
		return false;
	}

	ClearActionMessage();
	return true;
}

void UEDItemCraftingWidget::InitializeInventoryComponent()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: OwningPlayerPawn을 찾을 수 없습니다."));
		return;
	}

	InventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(OwningPawn);
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: InventoryComponent를 찾을 수 없습니다."));
	}
}

void UEDItemCraftingWidget::BindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UEDItemCraftingWidget::HandleInventoryChanged);
}

void UEDItemCraftingWidget::UnbindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UEDItemCraftingWidget::HandleInventoryChanged);
}

void UEDItemCraftingWidget::RefreshCraftRecipes()
{
	CachedRecipeViews.Reset();

	if (!InventoryComponent)
	{
		RebuildRecipeEntries();
		RebuildIngredientEntries();
		RebuildCraftTreeNodes();
		RefreshSelectedRecipeSummary();
		return;
	}

	UEDInventoryBlueprintLibrary::GetCraftRecipeViewDataList(
		InventoryComponent,
		CachedRecipeViews,
		false,
		EEDCraftableRecipeSortOption::ByRarity,
		true);

	if (SelectedRecipeRowId.IsNone() && CachedRecipeViews.Num() > 0)
	{
		SelectedRecipeRowId = CachedRecipeViews[0].RowId;
	}

	bool bHasSelectedRecipe = false;
	for (const FEDCraftRecipeViewData& RecipeView : CachedRecipeViews)
	{
		if (RecipeView.RowId == SelectedRecipeRowId)
		{
			bHasSelectedRecipe = true;
			break;
		}
	}

	if (!bHasSelectedRecipe)
	{
		SelectedRecipeRowId = CachedRecipeViews.Num() > 0 ? CachedRecipeViews[0].RowId : NAME_None;
	}

	RebuildRecipeEntries();
	RebuildIngredientEntries();
	RebuildCraftTreeNodes();
	RefreshSelectedRecipeSummary();
}

void UEDItemCraftingWidget::RebuildRecipeEntries()
{
	if (!RecipeListContainer)
	{
		return;
	}

	RecipeListContainer->ClearChildren();
	RecipeEntryWidgets.Reset();

	if (!RecipeEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: RecipeEntryWidgetClass가 설정되지 않았습니다."));
		return;
	}

	for (const FEDCraftRecipeViewData& RecipeView : CachedRecipeViews)
	{
		UEDCraftRecipeEntryWidget* EntryWidget = CreateWidget<UEDCraftRecipeEntryWidget>(this, RecipeEntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetRecipeViewData(RecipeView);
		EntryWidget->SetSelectedState(RecipeView.RowId == SelectedRecipeRowId);
		EntryWidget->OnRecipeEntryClicked.AddUObject(this, &UEDItemCraftingWidget::HandleRecipeEntryClicked);

		RecipeListContainer->AddChild(EntryWidget);
		RecipeEntryWidgets.Add(EntryWidget);
	}
}

void UEDItemCraftingWidget::RebuildIngredientEntries()
{
	if (!IngredientListContainer)
	{
		return;
	}

	IngredientListContainer->ClearChildren();
	IngredientEntryWidgets.Reset();

	if (!IngredientEntryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: IngredientEntryWidgetClass가 설정되지 않았습니다."));
		return;
	}

	FEDCraftRecipeViewData SelectedRecipe;
	if (!TryGetSelectedRecipeViewData(SelectedRecipe))
	{
		return;
	}

	for (const FEDCraftIngredientViewData& IngredientView : SelectedRecipe.Ingredients)
	{
		UEDCraftIngredientEntryWidget* EntryWidget = CreateWidget<UEDCraftIngredientEntryWidget>(this, IngredientEntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetIngredientViewData(IngredientView);
		IngredientListContainer->AddChild(EntryWidget);
		IngredientEntryWidgets.Add(EntryWidget);
	}
}

void UEDItemCraftingWidget::RebuildCraftTreeNodes()
{
	if (!CraftTreeContainer)
	{
		return;
	}

	CraftTreeContainer->ClearChildren();
	CraftTreeNodeWidgets.Reset();

	if (!CraftTreeNodeWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: CraftTreeNodeWidgetClass가 설정되지 않았습니다."));
		return;
	}

	FEDCraftRecipeViewData SelectedRecipe;
	if (!TryGetSelectedRecipeViewData(SelectedRecipe))
	{
		return;
	}

	TArray<FEDCraftTreeFlatNode> FlatNodes;
	bool bCyclePruned = false;
	UEDInventoryBlueprintLibrary::BuildCraftTreeFlat(
		InventoryComponent,
		SelectedRecipe.ResultItemId,
		FlatNodes,
		bCyclePruned,
		8);

	if (FlatNodes.Num() <= 0 || !WidgetTree)
	{
		return;
	}

	FlatNodes.Sort([](const FEDCraftTreeFlatNode& A, const FEDCraftTreeFlatNode& B)
	{
		if (A.Depth != B.Depth)
		{
			return A.Depth < B.Depth;
		}

		return A.NodeId < B.NodeId;
	});

	int32 CurrentDepth = INDEX_NONE;
	UHorizontalBox* CurrentRow = nullptr;

	for (const FEDCraftTreeFlatNode& FlatNode : FlatNodes)
	{
		if (CurrentDepth != FlatNode.Depth)
		{
			CurrentDepth = FlatNode.Depth;
			CurrentRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
			if (!CurrentRow)
			{
				continue;
			}

			CraftTreeContainer->AddChild(CurrentRow);
		}

		if (!CurrentRow)
		{
			continue;
		}

		FEDCraftTreeNodeViewData NodeViewData;
		if (!BuildCraftTreeNodeViewData(FlatNode, NodeViewData))
		{
			continue;
		}

		UEDCraftTreeNodeWidget* NodeWidget = CreateWidget<UEDCraftTreeNodeWidget>(this, CraftTreeNodeWidgetClass);
		if (!NodeWidget)
		{
			continue;
		}

		NodeWidget->SetTreeNodeViewData(NodeViewData);
		CurrentRow->AddChildToHorizontalBox(NodeWidget);
		CraftTreeNodeWidgets.Add(NodeWidget);
	}
}

void UEDItemCraftingWidget::RefreshSelectedRecipeSummary()
{
	FEDCraftRecipeViewData SelectedRecipe;
	if (!TryGetSelectedRecipeViewData(SelectedRecipe))
	{
		if (SelectedRecipeIconImage)
		{
			SelectedRecipeIconImage->SetBrushFromTexture(nullptr);
		}

		if (SelectedRecipeNameText)
		{
			SelectedRecipeNameText->SetText(FText::FromString(TEXT("No Recipe Selected")));
		}

		if (SelectedRecipeStateText)
		{
			SelectedRecipeStateText->SetText(FText::GetEmpty());
		}

		return;
	}

	if (SelectedRecipeIconImage)
	{
		SelectedRecipeIconImage->SetBrushFromTexture(SelectedRecipe.ResultIconTexture);
	}

	if (SelectedRecipeNameText)
	{
		SelectedRecipeNameText->SetText(SelectedRecipe.ResultItemName);
	}

	if (SelectedRecipeStateText)
	{
		SelectedRecipeStateText->SetText(
			SelectedRecipe.bCanCraft
				? FText::FromString(TEXT("Press Craft Key to craft"))
				: FText::FromString(TEXT("Collect all required materials")));
	}
}

bool UEDItemCraftingWidget::TryGetSelectedRecipeViewData(FEDCraftRecipeViewData& OutRecipeData) const
{
	for (const FEDCraftRecipeViewData& RecipeView : CachedRecipeViews)
	{
		if (RecipeView.RowId == SelectedRecipeRowId)
		{
			OutRecipeData = RecipeView;
			return true;
		}
	}

	return false;
}

bool UEDItemCraftingWidget::BuildCraftTreeNodeViewData(const FEDCraftTreeFlatNode& InFlatNode, FEDCraftTreeNodeViewData& OutNodeData) const
{
	OutNodeData = FEDCraftTreeNodeViewData();

	if (!InFlatNode.ItemId.IsValid())
	{
		return false;
	}

	const UEDInventoryItemDataAsset* ItemData = ResolveCraftTreeItemData(InFlatNode.ItemId);

	OutNodeData.NodeId = InFlatNode.NodeId;
	OutNodeData.ParentNodeId = InFlatNode.ParentNodeId;
	OutNodeData.Depth = InFlatNode.Depth;
	OutNodeData.ItemId = InFlatNode.ItemId;
	OutNodeData.DisplayName = ItemData && !ItemData->DisplayName.IsEmpty()
		? ItemData->DisplayName
		: FText::FromName(InFlatNode.ItemId.PrimaryAssetName);
	OutNodeData.Rarity = ItemData ? ItemData->Rarity : EEDItemRarity::Normal;
	OutNodeData.IconTexture = ItemData ? ItemData->IconTexture : nullptr;
	OutNodeData.RequiredQuantity = FMath::Max(1, InFlatNode.Quantity);
	OutNodeData.OwnedQuantity = CountOwnedItemQuantity(InFlatNode.ItemId);
	OutNodeData.bSatisfied = OutNodeData.OwnedQuantity >= OutNodeData.RequiredQuantity;
	OutNodeData.bIsCraftable = InFlatNode.bIsCraftable;
	return true;
}

int32 UEDItemCraftingWidget::CountOwnedItemQuantity(const FPrimaryAssetId& ItemId) const
{
	if (!InventoryComponent || !ItemId.IsValid())
	{
		return 0;
	}

	int32 OwnedQuantity = 0;

	for (const FEDInventorySlotData& SlotData : InventoryComponent->InventorySlots)
	{
		if (!SlotData.IsEmpty() && SlotData.Item.ItemId == ItemId)
		{
			OwnedQuantity += SlotData.Item.Quantity;
		}
	}

	const TArray<const FEDEquipmentSlotData*> EquipmentSlots =
	{
		&InventoryComponent->WeaponSlot,
		&InventoryComponent->TopArmorSlot,
		&InventoryComponent->BottomArmorSlot
	};

	for (const FEDEquipmentSlotData* EquipmentSlot : EquipmentSlots)
	{
		if (EquipmentSlot && EquipmentSlot->EquippedItem.IsValid() && EquipmentSlot->EquippedItem.ItemId == ItemId)
		{
			OwnedQuantity += EquipmentSlot->EquippedItem.Quantity;
		}
	}

	return OwnedQuantity;
}

void UEDItemCraftingWidget::HandleRecipeEntryClicked(FName InRecipeRowId)
{
	if (InRecipeRowId.IsNone())
	{
		return;
	}

	SelectedRecipeRowId = InRecipeRowId;
	RebuildRecipeEntries();
	RebuildIngredientEntries();
	RebuildCraftTreeNodes();
	RefreshSelectedRecipeSummary();
}

void UEDItemCraftingWidget::HandleInventoryChanged()
{
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::ShowCraftFailure(EEDInventoryActionFailure Failure) const
{
	if (!ActionResultText)
	{
		return;
	}

	ActionResultText->SetText(UEDInventoryBlueprintLibrary::GetInventoryActionFailureText(Failure));
	ActionResultText->SetVisibility(ESlateVisibility::Visible);
}

void UEDItemCraftingWidget::ClearActionMessage() const
{
	if (!ActionResultText)
	{
		return;
	}

	ActionResultText->SetText(FText::GetEmpty());
	ActionResultText->SetVisibility(ESlateVisibility::Collapsed);
}
