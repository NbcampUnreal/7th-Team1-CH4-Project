// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDSkillSlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "UI/EDInventoryDragDropOperation.h"

namespace
{
FText FormatCooldownText(float InRemainingTime)
{
	return FText::AsNumber(FMath::CeilToInt(InRemainingTime));
}
}

void UEDSkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ClearCooldown();
}

void UEDSkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 쿨타임 중일 때만 남은 시간 숫자를 갱신
	if (!bIsOnCooldown)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		ClearCooldown();
		return;
	}

	const float RemainingTime = FMath::Max(0.0f, CooldownEndWorldTime - World->GetTimeSeconds());
	if (RemainingTime <= KINDA_SMALL_NUMBER)
	{
		ClearCooldown();
		return;
	}

	if (CooldownValueText)
	{
		CooldownValueText->SetText(FormatCooldownText(RemainingTime));
	}
}

void UEDSkillSlotWidget::SetSlotDisplayData(const FEDSkillSlotDisplayData& InDisplayData)
{
	CachedIconTexture = InDisplayData.IconTexture;
	CachedKeyLabel = InDisplayData.KeyLabel;
	CachedSkillTag = InDisplayData.SkillTag;

	if (SkillIconImage)
	{
		SkillIconImage->SetVisibility(CachedIconTexture ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		SkillIconImage->SetBrushFromTexture(CachedIconTexture);
	}

	if (KeyText)
	{
		KeyText->SetText(CachedKeyLabel);
	}
}

void UEDSkillSlotWidget::StartCooldown(float InRemainingTime, float InMaxCooldownTime)
{
	const UWorld* World = GetWorld();
	if (!World || InRemainingTime <= KINDA_SMALL_NUMBER || InMaxCooldownTime <= KINDA_SMALL_NUMBER)
	{
		ClearCooldown();
		return;
	}

	bIsOnCooldown = true;
	CooldownStartWorldTime = World->GetTimeSeconds();
	CooldownEndWorldTime = CooldownStartWorldTime + InRemainingTime;
	CooldownDuration = InMaxCooldownTime;

	if (CooldownMaskSizeBox)
	{
		CooldownMaskSizeBox->ClearHeightOverride();
	}

	if (CooldownOverlayBorder)
	{
		CooldownOverlayBorder->SetVisibility(ESlateVisibility::Visible);
	}

	if (CooldownValueText)
	{
		CooldownValueText->SetVisibility(ESlateVisibility::Visible);
		CooldownValueText->SetText(FormatCooldownText(InRemainingTime));
	}
}

void UEDSkillSlotWidget::ClearCooldown()
{
	bIsOnCooldown = false;
	CooldownStartWorldTime = 0.0f;
	CooldownEndWorldTime = 0.0f;
	CooldownDuration = 0.0f;

	if (CooldownMaskSizeBox)
	{
		CooldownMaskSizeBox->ClearHeightOverride();
	}

	if (CooldownOverlayBorder)
	{
		CooldownOverlayBorder->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (CooldownValueText)
	{
		CooldownValueText->SetVisibility(ESlateVisibility::Collapsed);
		CooldownValueText->SetText(FText::GetEmpty());
	}
}

void UEDSkillSlotWidget::SetTargetSkillSlotType(EEDSkillSlotType InTargetSkillSlotType)
{
	TargetSkillSlotType = InTargetSkillSlotType;
}

FReply UEDSkillSlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnSkillSlotDoubleClicked.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

bool UEDSkillSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UEDInventoryDragDropOperation* DragOperation = Cast<UEDInventoryDragDropOperation>(InOperation);
	if (!DragOperation || DragOperation->SourceSlotIndex == INDEX_NONE)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	// 인벤토리 슬롯에서 시작한 드래그만 받아 상위 위젯에 장착 요청을 넘김
	DragOperation->bHandledByDropTarget = true;
	OnSkillSlotDropped.Broadcast(DragOperation->SourceSlotIndex);
	return true;
}
