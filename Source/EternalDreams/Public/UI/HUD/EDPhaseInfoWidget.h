#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "EDPhaseInfoWidget.generated.h"

class UTextBlock;
class AEDGameState;

UCLASS()
class ETERNALDREAMS_API UEDPhaseInfoWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// "Day 1" 형태의 일차 표시
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PhaseInfo")
	TObjectPtr<UTextBlock> DayText;

	// "Day" / "Night" 라벨
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PhaseInfo")
	TObjectPtr<UTextBlock> PhaseText;

	// "MM:SS" 남은 시간
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PhaseInfo")
	TObjectPtr<UTextBlock> TimeText;

	// 남은 시간 텍스트 갱신 주기(초)
	UPROPERTY(EditDefaultsOnly, Category = "PhaseInfo")
	float RefreshInterval = 0.25f;

private:
	TWeakObjectPtr<AEDGameState> CachedGameState;
	FTimerHandle RefreshTimerHandle;

	void TryCacheGameState();
	void BindPhaseChanged();
	void UnbindPhaseChanged();

	UFUNCTION()
	void HandlePhaseChanged(const FGameplayTag& OldPhase, const FGameplayTag& NewPhase);

	void RefreshPhaseLabels();
	void RefreshRemainingTime();
};