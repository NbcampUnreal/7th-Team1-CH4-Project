// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDSkillSlotWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

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

	RefreshCooldownOverlay(MyGeometry.GetLocalSize().Y);
}

void UEDSkillSlotWidget::SetSlotDisplayData(const FEDSkillSlotDisplayData& InDisplayData)
{
	CachedIconTexture = InDisplayData.IconTexture;
	CachedKeyLabel = InDisplayData.KeyLabel;
	CachedSkillTag = InDisplayData.SkillTag;

	if (SkillIconImage)
	{
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

	if (CooldownOverlayBorder)
	{
		CooldownOverlayBorder->SetVisibility(ESlateVisibility::Visible);
	}

	if (CooldownValueText)
	{
		CooldownValueText->SetVisibility(ESlateVisibility::Visible);
		CooldownValueText->SetText(FormatCooldownText(InRemainingTime));
	}

	RefreshCooldownOverlay(GetCachedGeometry().GetLocalSize().Y);
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

void UEDSkillSlotWidget::RefreshCooldownOverlay(float SlotHeight) const
{
	if (!bIsOnCooldown || !CooldownMaskSizeBox)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World || SlotHeight <= KINDA_SMALL_NUMBER || CooldownDuration <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float RemainingTime = FMath::Max(0.0f, CooldownEndWorldTime - World->GetTimeSeconds());
	const float CooldownPercent = FMath::Clamp(RemainingTime / CooldownDuration, 0.0f, 1.0f);
	CooldownMaskSizeBox->SetHeightOverride(SlotHeight * CooldownPercent);
}
