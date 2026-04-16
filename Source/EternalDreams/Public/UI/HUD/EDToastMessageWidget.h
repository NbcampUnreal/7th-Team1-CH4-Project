#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "UI/Types/EDUITypes.h"
#include "EDToastMessageWidget.generated.h"

class UBorder;
class UTextBlock;

/**
 * 짧은 시간 동안 나타났다 사라지는 알림/경고 메시지 위젯
 * HUD의 지정된 위치에 하나만 두고, 들어오는 메시지를 순차적으로 교체해서 사용
 */
UCLASS()
class ETERNALDREAMS_API UEDToastMessageWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 지정한 메시지를 표시하고, 일정 시간이 지나면 자연스럽게 사라지게 만듦
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ShowToastMessage(const FText& InMessage, EEDUIMessageType InMessageType, float InDuration = 3.0f);

protected:
	// 메시지 배경 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Toast")
	TObjectPtr<UBorder> BackgroundBorder;

	// 실제 메시지 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Toast")
	TObjectPtr<UTextBlock> MessageText;

private:
	FLinearColor GetBackgroundColor(EEDUIMessageType InMessageType) const;
	FLinearColor GetTextColor(EEDUIMessageType InMessageType) const;

private:
	bool bIsShowing = false;
	float ElapsedTime = 0.0f;
	float DisplayDuration = 3.0f;
	float FadeInDuration = 0.18f;
	float FadeOutDuration = 0.35f;
};
