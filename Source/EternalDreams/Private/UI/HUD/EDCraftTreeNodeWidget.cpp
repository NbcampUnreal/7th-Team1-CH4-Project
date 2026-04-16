// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftTreeNodeWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UEDCraftTreeNodeWidget::SetTreeNodeViewData(const FEDCraftTreeNodeViewData& InNodeData)
{
	if (ItemIconImage)
	{
		ItemIconImage->SetBrushFromTexture(InNodeData.IconTexture);
	}

	if (ItemNameText)
	{
		ItemNameText->SetText(InNodeData.DisplayName);
	}

	if (QuantityText)
	{
		QuantityText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), InNodeData.OwnedQuantity, InNodeData.RequiredQuantity)));
	}

	if (RarityAccent)
	{
		RarityAccent->SetBrushColor(GetRarityColor(InNodeData.Rarity));
	}

	if (SatisfiedBorder)
	{
		SatisfiedBorder->SetBrushColor(
			InNodeData.bSatisfied
				? FLinearColor(0.20f, 0.85f, 0.35f, 1.00f)
				: FLinearColor(0.85f, 0.25f, 0.25f, 1.00f));
	}
}

FLinearColor UEDCraftTreeNodeWidget::GetRarityColor(EEDItemRarity InRarity) const
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
