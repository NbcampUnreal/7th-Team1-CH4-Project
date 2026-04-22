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
	// 상단 정보 바에서 현재 일차를 표시
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PhaseInfo")
	TObjectPtr<UTextBlock> DayText;

	// 상단 정보 바에서 현재 낮/밤 여부를 표시
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PhaseInfo")
	TObjectPtr<UTextBlock> PhaseText;

	// 상단 정보 바에서 현재 Phase 기준 남은 시간을 표시
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PhaseInfo")
	TObjectPtr<UTextBlock> TimeText;

	// GameState가 늦게 준비되거나 시간이 흐를 때를 대비해 표시를 주기적으로 갱신
	UPROPERTY(EditDefaultsOnly, Category = "PhaseInfo")
	float RefreshInterval = 0.25f;

private:
	TWeakObjectPtr<AEDGameState> CachedGameState;
	FTimerHandle RefreshTimerHandle;

	// 현재 월드에서 GameState를 다시 찾음
	void TryCacheGameState();

	// Phase 변경 델리게이트를 구독해 일차 / 낮밤 표시를 즉시 갱신
	void BindPhaseChanged();

	// 위젯이 내려갈 때 Phase 변경 델리게이트를 해제
	void UnbindPhaseChanged();

	UFUNCTION()
	void HandlePhaseChanged(const FGameplayTag& OldPhase, const FGameplayTag& NewPhase);

	// 상단 정보 바의 모든 텍스트를 현재 GameState 기준으로 다시 그림
	void RefreshDisplay();

	// 일차 / 낮밤 표시를 갱신
	void RefreshPhaseLabels();

	// 남은 시간을 MM:SS 형식으로 갱신
	void RefreshRemainingTime();

	// GameState가 아직 준비되지 않았을 때 기본 표시를 채움
	void ApplyDefaultTexts() const;
};
