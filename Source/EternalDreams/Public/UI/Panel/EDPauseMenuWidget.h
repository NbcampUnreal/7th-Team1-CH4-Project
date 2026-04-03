#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDPauseMenuWidget.generated.h"

class UTextBlock;

UCLASS()
class ETERNALDREAMS_API UEDPauseMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	// 제목 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Pause")
	TObjectPtr<UTextBlock> TitleText;

	// 설명 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Pause")
	TObjectPtr<UTextBlock> DescriptionText;

private:
	void ApplyPreviewText() const;
};
