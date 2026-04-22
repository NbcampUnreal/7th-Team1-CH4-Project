// Copyright Epic Games, Inc. All Rights Reserved.s
#include "UI/Panel/EDInventorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "UI/EDInventoryDragDropOperation.h"
#include "UI/Style/EDUIRarityColors.h"

void UEDInventorySlotWidget::SetEmptyState()
{
	bHasItem = false;
	CurrentQuantity = 0;
	CurrentItemName = FText::GetEmpty();
	CurrentRarity = EEDItemRarity::Normal;
	CurrentIconTexture = nullptr;

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

	if (ItemIconImage)
	{
		ItemIconImage->SetVisibility(ESlateVisibility::Collapsed);
		ItemIconImage->SetBrushFromTexture(nullptr);
	}
}

void UEDInventorySlotWidget::SetItemState(const FText& InItemName, int32 InQuantity, EEDItemRarity InRarity, UTexture2D* InIconTexture)
{
	bHasItem = true;
	CurrentQuantity = InQuantity;
	CurrentItemName = InItemName;
	CurrentRarity = InRarity;
	CurrentIconTexture = InIconTexture;

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

	if (ItemIconImage)
	{
		ItemIconImage->SetVisibility(InIconTexture ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		ItemIconImage->SetBrushFromTexture(InIconTexture);
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

void UEDInventorySlotWidget::SetSourceInventoryComponent(UEDInventoryComponent* InSourceInventoryComponent)
{
	SourceInventoryComponent = InSourceInventoryComponent;
}

void UEDInventorySlotWidget::SetSupportsItemDrag(bool bInSupportsItemDrag)
{
	bSupportsItemDrag = bInSupportsItemDrag;
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

		if (bSupportsItemDrag && bHasItem && CurrentQuantity > 0 && SourceInventoryComponent)
		{
			return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		}

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

void UEDInventorySlotWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (SlotIndex == INDEX_NONE || !bSupportsItemDrag || !bHasItem || CurrentQuantity <= 0 || !SourceInventoryComponent)
	{
		return;
	}

	UEDInventoryDragDropOperation* DragOperation = NewObject<UEDInventoryDragDropOperation>(this);
	if (!DragOperation)
	{
		return;
	}

	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->SourceInventoryComponent = SourceInventoryComponent;
	DragOperation->SourceItemName = CurrentItemName;
	DragOperation->SourceItemQuantity = CurrentQuantity;
	DragOperation->SourceItemRarity = CurrentRarity;
	DragOperation->SourceIconTexture = CurrentIconTexture;
	DragOperation->Pivot = EDragPivot::MouseDown;

	// 드래그 비주얼은 현재 슬롯 위젯 클래스를 그대로 사용한다.
	if (UUserWidget* DragVisual = CreateWidget<UUserWidget>(this, GetClass()))
	{
		if (UEDInventorySlotWidget* DragVisualSlot = Cast<UEDInventorySlotWidget>(DragVisual))
		{
			DragVisualSlot->SetItemState(CurrentItemName, CurrentQuantity, CurrentRarity, CurrentIconTexture);
			DragVisualSlot->SetSelectedState(false);
			DragVisualSlot->SetSupportsItemDrag(false);
			DragVisualSlot->SetSourceInventoryComponent(nullptr);
		}

		DragOperation->DefaultDragVisual = DragVisual;
	}

	OutOperation = DragOperation;
}
