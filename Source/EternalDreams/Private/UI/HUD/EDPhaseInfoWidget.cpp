#include "UI/HUD/EDPhaseInfoWidget.h"

#include "Components/TextBlock.h"
#include "Core/EDGameState.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "EDPhaseInfoWidget"

void UEDPhaseInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	TryCacheGameState();
	BindPhaseChanged();
	RefreshPhaseLabels();
	RefreshRemainingTime();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			FTimerDelegate::CreateUObject(this, &UEDPhaseInfoWidget::RefreshRemainingTime),
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
	if (CachedGameState.IsValid()) return;

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
	RefreshPhaseLabels();
	RefreshRemainingTime();
}

void UEDPhaseInfoWidget::RefreshPhaseLabels()
{
	TryCacheGameState();
	if (!CachedGameState.IsValid()) return;

	const int32 Day = CachedGameState->GetCurrentDay();
	const bool bNight = CachedGameState->GetIsNight();

	if (DayText)
	{
		DayText->SetText(FText::Format(LOCTEXT("DayFmt", "Day {0}"), FText::AsNumber(Day)));
	}
	if (PhaseText)
	{
		PhaseText->SetText(bNight ? LOCTEXT("Night", "Night") : LOCTEXT("Day", "Day"));
	}
}

void UEDPhaseInfoWidget::RefreshRemainingTime()
{
	TryCacheGameState();
	if (!CachedGameState.IsValid() || !TimeText) return;

	const float Remaining = FMath::Max(0.f, CachedGameState->GetPhaseRemainingTime());
	const int32 Minutes = FMath::FloorToInt(Remaining / 60.f);
	const int32 Seconds = FMath::FloorToInt(FMath::Fmod(Remaining, 60.f));

	TimeText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
}

#undef LOCTEXT_NAMESPACE