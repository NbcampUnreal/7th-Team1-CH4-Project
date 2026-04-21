// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDQuickBarSlotWidget.h"

#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "UI/EDInventoryDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UEDQuickBarSlotWidget::SetSlotIndex(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;
}

void UEDQuickBarSlotWidget::SetEmptyState()
{
	bHasItem = false;
	CurrentQuantity = 0;

	Super::SetEmptyState();
}

void UEDQuickBarSlotWidget::SetItemState(const FText& InItemName, int32 InQuantity, EEDItemRarity InRarity)
{
	bHasItem = true;
	CurrentQuantity = InQuantity;

	Super::SetItemState(InItemName, InQuantity, InRarity);
}

FReply UEDQuickBarSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SlotIndex == INDEX_NONE || !bHasItem || CurrentQuantity <= 0)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	// 좌클릭 - 슬롯 선택 + 드래그 감지 시작
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnQuickBarSlotClicked.Broadcast(SlotIndex);
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UEDQuickBarSlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
                                                             const FPointerEvent& InMouseEvent)
{
	if (SlotIndex == INDEX_NONE)
	{
		return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
	}

	// 왼쪽 버튼 더블 클릭 - 소비 아이템 즉시 사용 또는 장비 아이템 장착
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnQuickBarSlotDoubleClicked.Broadcast(SlotIndex);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UEDQuickBarSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                 UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (SlotIndex == INDEX_NONE || !bHasItem || CurrentQuantity <= 0)
	{
		return;
	}

	// 좌클릭 드래그 시작 시 슬롯 인덱스를 담은 오퍼레이션을 만듦
	UEDInventoryDragDropOperation* DragOperation = NewObject<UEDInventoryDragDropOperation>(this);
	if (!DragOperation)
	{
		return;
	}

	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->SourceInventoryComponent = GetSourceInventoryComponent();
	DragOperation->Pivot = EDragPivot::MouseDown;
	
	// 드래그 중 마우스를 따라다닐 비주얼 위젯을 생성
	if (DragVisualWidgetClass)
	{
		if (UUserWidget* DragVisual = CreateWidget<UUserWidget>(this, DragVisualWidgetClass))
		{
			DragOperation->DefaultDragVisual = DragVisual;
		}
	}

	OutOperation = DragOperation;
}

bool UEDQuickBarSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                                         UDragDropOperation* InOperation)
{
	if (SlotIndex == INDEX_NONE || !InOperation)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	UEDInventoryDragDropOperation* DragOperation = Cast<UEDInventoryDragDropOperation>(InOperation);
	if (!DragOperation)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	if (DragOperation->SourceSlotIndex == INDEX_NONE)
	{
		return true;
	}

	// 같은 슬롯에 드롭한 경우는 무시
	if (DragOperation->SourceSlotIndex == SlotIndex)
	{
		return true;
	}

	// 다른 슬롯 위에 드롭되면 슬롯 이동 요청을 상위 퀵바로 넘김
	OnQuickBarSlotDroppedOnSlot.Broadcast(DragOperation->SourceInventoryComponent, DragOperation->SourceSlotIndex, SlotIndex);
	return true;
}

void UEDQuickBarSlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent,
                                                  UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	UEDInventoryDragDropOperation* DragOperation = Cast<UEDInventoryDragDropOperation>(InOperation);
	if (!DragOperation)
	{
		return;
	}

	if (DragOperation->SourceSlotIndex == INDEX_NONE)
	{
		return;
	}

	// 슬롯 위에 정상 드롭되지 않고 취소된 경우,
	// 퀵바 밖으로 버리기로 판단
	OnQuickBarSlotDroppedOutside.Broadcast(DragOperation->SourceSlotIndex);
}
