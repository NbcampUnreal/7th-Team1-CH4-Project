// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "TimerManager.h"
#include "EDMatchResultWidget.generated.h"

class UTextBlock;
class UWidgetSwitcher;

UCLASS()
class ETERNALDREAMS_API UEDMatchResultWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ED|Match")
	void SetResult(const TArray<int32>& TeamRankings, int32 MyTeamId);

	UFUNCTION(BlueprintCallable, Category = "ED|Match")
	void StartCountdown(float Seconds);

protected:
	virtual void NativeOnDeactivated() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "ED|Match")
	void OnResultSet(bool bIsVictory, int32 MyRank);

	UFUNCTION(BlueprintImplementableEvent, Category = "ED|Match")
	void OnCountdownUpdated(int32 WholeSeconds);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ResultSwitcher;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MyRankText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CountdownText;

private:
	void TickCountdown();
	void UpdateCountdownText();

	float RemainingSeconds = 0.f;
	FTimerHandle CountdownTimerHandle;
};
