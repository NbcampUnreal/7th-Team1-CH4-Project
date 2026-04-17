// Copyright Epic Games, Inc. All Rights Reserved.s
#include "UI/Panel/EDInventorySlotWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/Core/EDUIRarityColors.h"

void UEDInventorySlotWidget::SetEmptyState()
{
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
		RarityAccent->SetBrushColor(EDRarityColors::Resolve(InRarity));
	}
}

void UEDInventorySlotWidget::SetSlotIndex(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
}

void UEDInventorySlotWidget::SetSelectedState(bool bSelected)
{
	bIsSelected = bSelected;

	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

FReply UEDInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SlotIndex == INDEX_NONE)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnSlotClicked.Broadcast(SlotIndex);
		return FReply::Handled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OnSlotRightClicked.Broadcast(SlotIndex);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UEDInventorySlotWidget::NativeOnMouseButtonDoubleClick(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (SlotIndex == INDEX_NONE)
	{
		return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnSlotDoubleClicked.Broadcast(SlotIndex);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}
