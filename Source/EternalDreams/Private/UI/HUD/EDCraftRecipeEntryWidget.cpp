// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftRecipeEntryWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "UI/Core/EDUIRarityColors.h"

void UEDCraftRecipeEntryWidget::SetRecipeEntryData(const FEDCraftRecipeEntryDisplayData& InDisplayData)
{
	RecipeRowId = InDisplayData.RowId;

	if (RecipeIconImage)
	{
		RecipeIconImage->SetBrushFromTexture(InDisplayData.ResultIconTexture);
	}

	if (RecipeNameText)
	{
		RecipeNameText->SetText(InDisplayData.ResultItemName);
	}

	if (CraftStateText)
	{
		CraftStateText->SetText(InDisplayData.bCanCraft
			? FText::FromString(TEXT("제작 가능"))
			: FText::FromString(TEXT("제작 불가")));
		CraftStateText->SetColorAndOpacity(
			InDisplayData.bCanCraft
				? FSlateColor(FLinearColor(0.20f, 0.85f, 0.35f, 1.00f))
				: FSlateColor(FLinearColor(0.80f, 0.80f, 0.80f, 1.00f)));
	}

	if (RarityAccent)
	{
		RarityAccent->SetBrushColor(EDRarityColors::Resolve(InDisplayData.ResultRarity));
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
