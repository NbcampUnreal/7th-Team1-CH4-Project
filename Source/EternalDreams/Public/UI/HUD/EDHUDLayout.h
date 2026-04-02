#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "EDHUDLayout.generated.h"

class UPanelWidget;

UCLASS()
class ETERNALDREAMS_API UEDHUDLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;

	// HUD 루트를 다시 표시할 때 호출
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowLayout();

	// HUD 루트를 숨길 때 호출
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideLayout();

protected:
	// HUD의 최상위 컨테이너 바인딩
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD")
	TObjectPtr<UPanelWidget> RootContainer;
};
