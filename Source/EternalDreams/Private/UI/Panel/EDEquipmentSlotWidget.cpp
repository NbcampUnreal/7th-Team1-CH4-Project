// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDEquipmentSlotWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "UI/Core/EDUIRarityColors.h"

void UEDEquipmentSlotWidget::SetEmptyState(const FText& InSlotTypeName)
{
	if (SlotTypeText)
	{
		SlotTypeText->SetText(InSlotTypeName);
		SlotTypeText->SetVisibility(ESlateVisibility::Visible);
	}

	if (EmptyText)
	{
		EmptyText->SetVisibility(ESlateVisibility::Visible);
		EmptyText->SetText(FText::FromString(TEXT("Empty")));
	}

	if (ItemNameText)
	{
		ItemNameText->SetVisibility(ESlateVisibility::Collapsed);
		ItemNameText->SetText(FText::GetEmpty());
	}

	if (RarityAccent)
	{
		RarityAccent->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEDEquipmentSlotWidget::SetItemState(const FText& InSlotTypeName, const FText& InItemName, EEDItemRarity InRarity)
{
	if (SlotTypeText)
	{
		SlotTypeText->SetText(InSlotTypeName);
		SlotTypeText->SetVisibility(ESlateVisibility::Visible);
	}

	if (EmptyText)
	{
		EmptyText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ItemNameText)
	{
		ItemNameText->SetVisibility(ESlateVisibility::Visible);
		ItemNameText->SetText(InItemName);
	}

	if (RarityAccent)
	{
		RarityAccent->SetVisibility(ESlateVisibility::Visible);
		RarityAccent->SetBrushColor(EDRarityColors::Resolve(InRarity));
	}
}

void UEDEquipmentSlotWidget::SetSlotType(EEDEquippableType InSlotType)
{
	SlotType = InSlotType;
}

void UEDEquipmentSlotWidget::SetSelectedState(bool bSelected)
{
	bIsSelected = bSelected;

	if (SelectionBorder)
	{
		SelectionBorder->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

FReply UEDEquipmentSlotWidget::NativeOnMouseButtonDoubleClick(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (SlotType == EEDEquippableType::None)
	{
		return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnEquipmentSlotDoubleClicked.Broadcast(SlotType);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply UEDEquipmentSlotWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (SlotType == EEDEquippableType::None)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnEquipmentSlotClicked.Broadcast(SlotType);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
