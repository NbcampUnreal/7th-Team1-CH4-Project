#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Types/EDUITypes.h"
#include "EDToastMessageWidget.generated.h"

class UBorder;
class USizeBox;
class UTextBlock;

/**
 * 짧은 시간 동안 나타났다가 자연스럽게 사라지는 토스트 메시지 위젯
 * HUD의 지정된 위치에 하나만 두고, 새 메시지가 오면 내용을 교체해서 사용
 */
UCLASS()
class ETERNALDREAMS_API UEDToastMessageWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 지정한 메시지를 표시하고, 일정 시간 뒤 fade-out으로 숨김
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ShowToastMessage(const FText& InMessage, EEDUIMessageType InMessageType, float InDuration = 3.0f);

protected:
	// 토스트 전체 폭을 제어하는 선택적 SizeBox
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Toast")
	TObjectPtr<USizeBox> RootSizeBox;

	// 메시지 배경 패널
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Toast")
	TObjectPtr<UBorder> BackgroundBorder;

	// 실제 메시지 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Toast")
	TObjectPtr<UTextBlock> MessageText;

private:
	void ApplyToastLayout();
	FLinearColor GetBackgroundColor(EEDUIMessageType InMessageType) const;
	FLinearColor GetTextColor(EEDUIMessageType InMessageType) const;

private:
	bool bIsShowing = false;
	float ElapsedTime = 0.0f;
	float DisplayDuration = 3.0f;
	float FadeInDuration = 0.18f;
	float FadeOutDuration = 0.35f;

	// 토스트 전체 폭을 조금 넉넉하게 잡아 긴 한글 메시지가 잘리지 않게 함
	float ToastWidth = 480.0f;

	// 텍스트는 배경 패딩을 제외한 내부 폭보다 더 좁게 줄바꿈해서 오른쪽이 잘리는 현상을 줄임
	float TextWrapWidth = 400.0f;
};
