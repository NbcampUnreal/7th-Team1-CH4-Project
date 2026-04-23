#include "UI/HUD/EDPhaseInfoWidget.h"

#include "Components/TextBlock.h"
#include "Core/EDGameState.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "EDPhaseInfoWidget"

namespace
{
	FText FormatRemainingTimeText(float InRemainingTime)
	{
		const float RemainingTime = FMath::Max(0.0f, InRemainingTime);
		const int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
		const int32 Seconds = FMath::FloorToInt(FMath::Fmod(RemainingTime, 60.0f));

		return FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
	}

	bool IsFinalPhase(const AEDGameState& GameState)
	{
		return GameState.GetCurrentPhase() == FEDGameplayTags::Get().Phase_Day4_Night;
	}
}

void UEDPhaseInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyDefaultTexts();
	TryCacheGameState();
	BindPhaseChanged();
	RefreshDisplay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			FTimerDelegate::CreateUObject(this, &UEDPhaseInfoWidget::RefreshDisplay),
			RefreshInterval,
			true);
	}
}

void UEDPhaseInfoWidget::NativeDestruct()
{
	UnbindPhaseChanged();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UEDPhaseInfoWidget::TryCacheGameState()
{
	if (CachedGameState.IsValid())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		CachedGameState = World->GetGameState<AEDGameState>();
	}
}

void UEDPhaseInfoWidget::BindPhaseChanged()
{
	TryCacheGameState();
	if (CachedGameState.IsValid())
	{
		CachedGameState->OnGamePhaseChanged.AddUniqueDynamic(this, &UEDPhaseInfoWidget::HandlePhaseChanged);
	}
}

void UEDPhaseInfoWidget::UnbindPhaseChanged()
{
	if (CachedGameState.IsValid())
	{
		CachedGameState->OnGamePhaseChanged.RemoveDynamic(this, &UEDPhaseInfoWidget::HandlePhaseChanged);
	}
}

void UEDPhaseInfoWidget::HandlePhaseChanged(const FGameplayTag& /*OldPhase*/, const FGameplayTag& /*NewPhase*/)
{
	RefreshDisplay();
}

void UEDPhaseInfoWidget::RefreshDisplay()
{
	TryCacheGameState();
	if (!CachedGameState.IsValid())
	{
		ApplyDefaultTexts();
		return;
	}

	RefreshPhaseLabels();
	RefreshRemainingTime();
}

void UEDPhaseInfoWidget::RefreshPhaseLabels()
{
	if (!CachedGameState.IsValid())
	{
		return;
	}

	const int32 CurrentDay = CachedGameState->GetCurrentDay();
	const bool bIsNight = CachedGameState->GetIsNight();

	if (DayText)
	{
		if (CurrentDay > 0)
		{
			DayText->SetText(FText::Format(LOCTEXT("DayFormat", "{0}일차"), FText::AsNumber(CurrentDay)));
		}
		else
		{
			DayText->SetText(LOCTEXT("DayDefault", "-일차"));
		}
	}

	if (PhaseText)
	{
		PhaseText->SetText(bIsNight ? LOCTEXT("NightLabel", "밤") : LOCTEXT("DayLabel", "낮"));
	}
}

void UEDPhaseInfoWidget::RefreshRemainingTime()
{
	if (!CachedGameState.IsValid() || !TimeText)
	{
		return;
	}

	const AEDGameState* GameState = CachedGameState.Get();
	if (IsFinalPhase(*GameState))
	{
		TimeText->SetText(LOCTEXT("FinalBattleLabel", "final"));
		return;
	}

	TimeText->SetText(FormatRemainingTimeText(GameState->GetPhaseRemainingTime()));
}

void UEDPhaseInfoWidget::ApplyDefaultTexts() const
{
	if (DayText)
	{
		DayText->SetText(LOCTEXT("DayFallback", "-일차"));
	}

	if (PhaseText)
	{
		PhaseText->SetText(LOCTEXT("PhaseFallback", "-"));
	}

	if (TimeText)
	{
		TimeText->SetText(LOCTEXT("TimeFallback", "--:--"));
	}
}

#undef LOCTEXT_NAMESPACE
