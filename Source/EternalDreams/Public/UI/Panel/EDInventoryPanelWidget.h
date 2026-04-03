#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDInventoryPanelWidget.generated.h"

class UTextBlock;

UCLASS()
class ETERNALDREAMS_API UEDInventoryPanelWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	// 제목 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> TitleText;

	// 설명 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> DescriptionText;

private:
	// 문구 확인 테스트용
	void ApplyPreviewText() const;
};
