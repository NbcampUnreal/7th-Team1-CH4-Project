// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDDeathOverlayWidget.generated.h"

class UTextBlock;
class UWidget;

/**
 * 사망 직후 Modal 레이어에 올라가는 오버레이 패널.
 * UIManager의 OpenPanel(Panel_DeathOverlay)로 열린다.
 * 부활 가능 시 CountdownSeconds 동안 1초 단위 카운트 후 스스로 ClosePanel 호출.
 * 부활 불가(Eliminated) 시 Eliminated 컨테이너 노출 (외부가 닫을 때까지 유지).
 */
UCLASS()
class ETERNALDREAMS_API UEDDeathOverlayWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ED|Death")
	void StartCountdown(float Seconds, bool bCanRespawn);

protected:
	virtual void NativeOnDeactivated() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CountdownText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CountdownContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> EliminatedContainer;

private:
	void TickCountdown();
	void UpdateCountdownText() const;
	void CloseSelfPanel();

	float RemainingSeconds = 0.f;
	FTimerHandle CountdownTimerHandle;
};
