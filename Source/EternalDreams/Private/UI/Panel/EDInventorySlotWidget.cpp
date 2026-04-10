// Copyright Epic Games, Inc. All Rights Reserved.s
#include "UI/Panel/EDInventorySlotWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"

void UEDInventorySlotWidget::SetEmptyState()
{
	// 빈 슬롯은 EmptyText만 보이고, 나머지 정보는 숨김 처리
	if (EmptyText)
	{
		EmptyText->SetVisibility(ESlateVisibility::Visible);
	}

	if (ItemNameText)
	{
		ItemNameText->SetVisibility(ESlateVisibility::Collapsed);
		ItemNameText->SetText(FText::GetEmpty());
	}

	if (QuantityText)
	{
		QuantityText->SetVisibility(ESlateVisibility::Collapsed);
		QuantityText->SetText(FText::GetEmpty());
	}

	if (RarityAccent)
	{
		RarityAccent->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEDInventorySlotWidget::SetItemState(const FText& InItemName, int32 InQuantity, EEDItemRarity InRarity)
{
	// 아이템이 있는 슬롯은 이름, 수량, 희귀도 라인을 표시
	if (EmptyText)
	{
		EmptyText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ItemNameText)
	{
		ItemNameText->SetVisibility(ESlateVisibility::Visible);
		ItemNameText->SetText(InItemName);
	}

	if (QuantityText)
	{
		QuantityText->SetVisibility(ESlateVisibility::Visible);
		QuantityText->SetText(FText::FromString(FString::Printf(TEXT("x%d"), InQuantity)));
	}

	if (RarityAccent)
	{
		RarityAccent->SetVisibility(ESlateVisibility::Visible);
		RarityAccent->SetBrushColor(GetRarityColor(InRarity));
	}
}

FLinearColor UEDInventorySlotWidget::GetRarityColor(EEDItemRarity InRarity) const
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
