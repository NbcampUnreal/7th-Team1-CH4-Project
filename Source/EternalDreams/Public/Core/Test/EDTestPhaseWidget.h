// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDTestPhaseWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * UEDTestPhaseWidget
 *
 * 페이즈 시스템 테스트용 UI.
 * 현재 페이즈, 남은 시간 표시 + 스킵 버튼.
 *
 * 사용법:
 *   에디터에서 이 클래스를 부모로 하는 WBP_TestPhase 위젯 블루프린트를 생성.
 *   BindWidget 변수명과 동일한 이름으로 UMG 컴포넌트를 배치.
 */
UCLASS()
class ETERNALDREAMS_API UEDTestPhaseWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// -------------------------------------------------------
	// UMG 바인딩
	// -------------------------------------------------------

	/** 현재 페이즈 이름 표시 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PhaseNameText;

	/** 남은 시간 표시 (MM:SS) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RemainingTimeText;

	/** 페이즈 인덱스 표시 (예: 1 / 8) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PhaseIndexText;

	/** 다음 페이즈 스킵 버튼 (테스트용) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkipPhaseButton;

	UFUNCTION()
	void OnSkipPhaseClicked();

private:
	void UpdateDisplay();
};
