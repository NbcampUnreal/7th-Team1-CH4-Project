// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftIngredientEntryWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEDCraftIngredientEntryWidget::SetIngredientViewData(const FEDCraftIngredientViewData& InIngredientData)
{
	if (IngredientIconImage)
	{
		IngredientIconImage->SetBrushFromTexture(InIngredientData.IconTexture);
	}

	if (IngredientNameText)
	{
		IngredientNameText->SetText(InIngredientData.DisplayName);
	}

	if (QuantityText)
	{
		QuantityText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), InIngredientData.OwnedQuantity, InIngredientData.RequiredQuantity)));
		QuantityText->SetColorAndOpacity(
			InIngredientData.bSatisfied
				? FSlateColor(FLinearColor(0.20f, 0.85f, 0.35f, 1.00f))
				: FSlateColor(FLinearColor(0.90f, 0.35f, 0.35f, 1.00f)));
	}

	if (SatisfiedAccent)
	{
		SatisfiedAccent->SetBrushColor(
			InIngredientData.bSatisfied
				? FLinearColor(0.20f, 0.85f, 0.35f, 1.00f)
				: FLinearColor(0.85f, 0.25f, 0.25f, 1.00f));
	}
}
