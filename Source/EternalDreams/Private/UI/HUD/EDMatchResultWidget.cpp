// Copyright Eternal Dreams Team. All Rights Reserved.

#include "UI/HUD/EDMatchResultWidget.h"

#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UEDMatchResultWidget::SetResult(const TArray<int32>& TeamRankings, int32 MyTeamId)
{
	int32 MyRank = INDEX_NONE;
	for (int32 i = 0; i < TeamRankings.Num(); ++i)
	{
		if (TeamRankings[i] == MyTeamId)
		{
			MyRank = i + 1;
			break;
		}
	}

	const bool bIsVictory = (MyRank == 1);

	if (ResultSwitcher)
	{
		ResultSwitcher->SetActiveWidgetIndex(bIsVictory ? 0 : 1);
	}

	if (MyRankText && MyRank != INDEX_NONE)
	{
		MyRankText->SetText(FText::FromString(FString::Printf(TEXT("#%d"), MyRank)));
	}

	OnResultSet(bIsVictory, MyRank);
}

void UEDMatchResultWidget::StartCountdown(float Seconds)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	RemainingSeconds = FMath::Max(Seconds, 0.f);
	UpdateCountdownText();

	if (RemainingSeconds <= 0.f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&UEDMatchResultWidget::TickCountdown,
			1.0f,
			true);
	}
}

void UEDMatchResultWidget::NativeOnDeactivated()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	Super::NativeOnDeactivated();
}

void UEDMatchResultWidget::TickCountdown()
{
	RemainingSeconds -= 1.f;
	UpdateCountdownText();

	if (RemainingSeconds <= 0.f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		}
	}
}

void UEDMatchResultWidget::UpdateCountdownText()
{
	const int32 WholeSeconds = FMath::Max(FMath::CeilToInt(RemainingSeconds), 0);

	if (CountdownText)
	{
		CountdownText->SetText(FText::AsNumber(WholeSeconds));
	}

	OnCountdownUpdated(WholeSeconds);
}
