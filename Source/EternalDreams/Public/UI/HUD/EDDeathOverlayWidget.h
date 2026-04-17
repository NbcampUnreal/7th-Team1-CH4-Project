// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDDeathOverlayWidget.generated.h"

class UTextBlock;
class UWidget;

/**
 * 사망 직후 뷰포트에 올라가는 오버레이.
 * 부활 가능 시 CountdownSeconds 동안 1초 단위 카운트 후 자동 제거.
 * 부활 불가(Eliminated) 시 Eliminated 컨테이너만 노출.
 */
UCLASS()
class ETERNALDREAMS_API UEDDeathOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ED|Death")
	void StartCountdown(float Seconds, bool bCanRespawn);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CountdownText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> CountdownContainer;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> EliminatedContainer;

private:
	void TickCountdown();
	void UpdateCountdownText() const;

	float RemainingSeconds = 0.f;
	FTimerHandle CountdownTimerHandle;
};
