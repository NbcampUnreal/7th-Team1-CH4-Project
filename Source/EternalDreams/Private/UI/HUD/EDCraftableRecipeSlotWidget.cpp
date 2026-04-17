// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftableRecipeSlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "UI/Core/EDUIRarityColors.h"

void UEDCraftableRecipeSlotWidget::SetSlotDisplayData(const FEDCraftableRecipeSlotDisplayData& InDisplayData)
{
	if (ItemIconImage)
	{
		ItemIconImage->SetBrushFromTexture(InDisplayData.IconTexture);
	}

	if (CurrentTargetBorder)
	{
		CurrentTargetBorder->SetBrushColor(EDRarityColors::Resolve(InDisplayData.Rarity));
		CurrentTargetBorder->SetVisibility(InDisplayData.bIsCurrentCraftTarget ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
