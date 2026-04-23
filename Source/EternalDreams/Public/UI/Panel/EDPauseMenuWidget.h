#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDPauseMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class ETERNALDREAMS_API UEDPauseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	// 계속하기 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Pause")
	TObjectPtr<UButton> ResumeButton;

	// 나가기 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Pause")
	TObjectPtr<UButton> QuitButton;

	// 제목 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Pause")
	TObjectPtr<UTextBlock> TitleText;
	
	UFUNCTION()
	void HandleResumeButtonClicked();

	UFUNCTION()
	void HandleQuitButtonClicked();

	// 일시정지 메뉴 닫기
	void ClosePauseMenu() const;
};
