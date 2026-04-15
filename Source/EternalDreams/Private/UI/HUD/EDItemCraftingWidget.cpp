// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDItemCraftingWidget.h"

#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "UI/HUD/EDCraftIngredientEntryWidget.h"
#include "UI/HUD/EDCraftRecipeEntryWidget.h"

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

void UEDItemCraftingWidget::RefreshSelectedRecipeSummary()
{
	FEDCraftRecipeViewData SelectedRecipe;
	if (!TryGetSelectedRecipeViewData(SelectedRecipe))
	{
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

	if (SelectedRecipeNameText)
	{
		SelectedRecipeNameText->SetText(SelectedRecipe.ResultItemName);
	}

	if (SelectedRecipeStateText)
	{
		SelectedRecipeStateText->SetText(
			SelectedRecipe.bCanCraft
				? FText::FromString(TEXT("Press Interaction Key to craft"))
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

void UEDItemCraftingWidget::HandleRecipeEntryClicked(FName InRecipeRowId)
{
	if (InRecipeRowId.IsNone())
	{
		return;
	}

	SelectedRecipeRowId = InRecipeRowId;
	RebuildRecipeEntries();
	RebuildIngredientEntries();
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
