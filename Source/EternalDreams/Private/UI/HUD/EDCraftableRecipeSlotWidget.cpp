// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDCraftableRecipeSlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "UI/Style/EDUIRarityColors.h"

void UEDCraftableRecipeSlotWidget::SetSlotDisplayData(const FEDCraftableRecipeSlotDisplayData& InDisplayData)
{
	// 각 슬롯 크기를 고정해 아이콘 원본 크기에 따라 슬롯이 찌그러지지 않게 한다.
	if (SlotSizeBox)
	{
		SlotSizeBox->SetWidthOverride(44.0f);
		SlotSizeBox->SetHeightOverride(44.0f);
	}

	if (ItemIconImage)
	{
		ItemIconImage->SetVisibility(InDisplayData.IconTexture ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		ItemIconImage->SetBrushFromTexture(InDisplayData.IconTexture);
		ItemIconImage->SetDesiredSizeOverride(FVector2D(36.0f, 36.0f));
	}

	if (CurrentTargetBorder)
	{
		CurrentTargetBorder->SetBrushColor(EDRarityColors::Resolve(InDisplayData.Rarity));
		CurrentTargetBorder->SetVisibility(InDisplayData.bIsCurrentCraftTarget ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
