#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "EDPlayerStatusWidget.generated.h"

class UTextBlock;
class UProgressBar;

UCLASS()
class ETERNALDREAMS_API UEDPlayerStatusWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 테스트용 HP 표시 바인딩용
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UProgressBar> HPBar;

	// 테스트용 텍스트 표시 바인딩용
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> StatusText;

private:
	// 더미 값 표시용
	void ApplyTestData() const;
};
