// Copyright Eternal Dreams Team. All Rights Reserved.

#include "UI/HUD/EDDeathOverlayWidget.h"

#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Types/EDUIWidgetIds.h"

void UEDDeathOverlayWidget::StartCountdown(float Seconds, bool bCanRespawn)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	if (bCanRespawn)
	{
		RemainingSeconds = FMath::Max(Seconds, 0.f);
		UpdateCountdownText();

		if (CountdownContainer)
		{
			CountdownContainer->SetVisibility(ESlateVisibility::Visible);
		}
		if (EliminatedContainer)
		{
			EliminatedContainer->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				CountdownTimerHandle, this, &UEDDeathOverlayWidget::TickCountdown, 1.0f, true);
		}
	}
	else
	{
		RemainingSeconds = 0.f;
		if (CountdownContainer)
		{
			CountdownContainer->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (EliminatedContainer)
		{
			EliminatedContainer->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UEDDeathOverlayWidget::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
	Super::NativeOnDeactivated();
}

void UEDDeathOverlayWidget::TickCountdown()
{
	RemainingSeconds -= 1.f;
	UpdateCountdownText();

	if (RemainingSeconds <= 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		}
		CloseSelfPanel();
	}
}

void UEDDeathOverlayWidget::UpdateCountdownText() const
{
	if (CountdownText)
	{
		const int32 WholeSeconds = FMath::CeilToInt(RemainingSeconds);
		CountdownText->SetText(FText::AsNumber(WholeSeconds));
	}
}

void UEDDeathOverlayWidget::CloseSelfPanel()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!LP)
	{
		return;
	}

	if (UEDUIManageSubsystem* UIMgr = LP->GetSubsystem<UEDUIManageSubsystem>())
	{
		UIMgr->ClosePanel(EDUIWidgetIds::Panel_DeathOverlay);
	}
}
