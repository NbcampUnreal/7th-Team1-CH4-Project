// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDEquipmentSlotWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"

void UEDEquipmentSlotWidget::SetEmptyState(const FText& InSlotTypeName)
{
	// 장비 슬롯 종류는 항상 보이도록 유지
	if (SlotTypeText)
	{
		SlotTypeText->SetText(InSlotTypeName);
		SlotTypeText->SetVisibility(ESlateVisibility::Visible);
	}

	// 빈 슬롯일 때 Empty 텍스트를 표시
	if (EmptyText)
	{
		EmptyText->SetVisibility(ESlateVisibility::Visible);
		EmptyText->SetText(FText::FromString(TEXT("Empty")));
	}

	// 장착된 아이템 이름은 숨김
	if (ItemNameText)
	{
		ItemNameText->SetVisibility(ESlateVisibility::Collapsed);
		ItemNameText->SetText(FText::GetEmpty());
	}

	// 희귀도 강조 라인은 숨김
	if (RarityAccent)
	{
		RarityAccent->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEDEquipmentSlotWidget::SetItemState(const FText& InSlotTypeName, const FText& InItemName, EEDItemRarity InRarity)
{
	// 슬롯 종류는 항상 보이도록 유지
	if (SlotTypeText)
	{
		SlotTypeText->SetText(InSlotTypeName);
		SlotTypeText->SetVisibility(ESlateVisibility::Visible);
	}

	// 장착된 아이템이 있으면 Empty 텍스트는 숨김
	if (EmptyText)
	{
		EmptyText->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 장착된 아이템 이름을 표시
	if (ItemNameText)
	{
		ItemNameText->SetVisibility(ESlateVisibility::Visible);
		ItemNameText->SetText(InItemName);
	}

	// 희귀도 강조 라인을 표시
	if (RarityAccent)
	{
		RarityAccent->SetVisibility(ESlateVisibility::Visible);
		RarityAccent->SetBrushColor(GetRarityColor(InRarity));
	}
}

FLinearColor UEDEquipmentSlotWidget::GetRarityColor(EEDItemRarity InRarity) const
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
