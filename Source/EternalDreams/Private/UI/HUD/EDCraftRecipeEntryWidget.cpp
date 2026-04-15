// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftRecipeEntryWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"

void UEDCraftRecipeEntryWidget::SetRecipeViewData(const FEDCraftRecipeViewData& InRecipeData)
{
	RecipeRowId = InRecipeData.RowId;

	if (RecipeNameText)
	{
		RecipeNameText->SetText(InRecipeData.ResultItemName);
	}

	if (CraftStateText)
	{
		CraftStateText->SetText(InRecipeData.bCanCraft
			? FText::FromString(TEXT("Craftable"))
			: FText::FromString(TEXT("Missing Materials")));
	}

	if (RarityAccent)
	{
		RarityAccent->SetBrushColor(GetRarityColor(InRecipeData.ResultRarity));
	}
}

void UEDCraftRecipeEntryWidget::SetSelectedState(bool bSelected)
{
	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

FReply UEDCraftRecipeEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !RecipeRowId.IsNone())
	{
		OnRecipeEntryClicked.Broadcast(RecipeRowId);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FLinearColor UEDCraftRecipeEntryWidget::GetRarityColor(EEDItemRarity InRarity) const
{
	switch (InRarity)
	{
	case EEDItemRarity::Rare:
		return FLinearColor(0.20f, 0.45f, 1.00f, 1.00f);
	case EEDItemRarity::Epic:
		return FLinearColor(0.65f, 0.25f, 1.00f, 1.00f);
	case EEDItemRarity::Legendary:
		return FLinearColor(1.00f, 0.55f, 0.10f, 1.00f);
	case EEDItemRarity::Unique:
		return FLinearColor(1.00f, 0.20f, 0.20f, 1.00f);
	case EEDItemRarity::Normal:
	default:
		return FLinearColor(0.65f, 0.65f, 0.65f, 1.00f);
	}
}
