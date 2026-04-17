// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftTreeNodeWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "UI/Core/EDUIRarityColors.h"

void UEDCraftTreeNodeWidget::SetTreeNodeDisplayData(const FEDCraftTreeNodeDisplayData& InDisplayData)
{
	if (ItemIconImage)
	{
		ItemIconImage->SetBrushFromTexture(InDisplayData.IconTexture);
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(InDisplayData.DisplayName);
	}

	if (QuantityText)
	{
		QuantityText->SetText(FText::AsNumber(FMath::Max(1, InDisplayData.RequiredQuantity)));
	}

	if (RarityAccent)
	{
		RarityAccent->SetBrushColor(EDRarityColors::Resolve(InDisplayData.Rarity));
	}
}
