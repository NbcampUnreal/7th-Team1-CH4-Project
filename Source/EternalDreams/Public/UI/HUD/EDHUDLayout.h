#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "EDHUDLayout.generated.h"

class UPanelWidget;
class UEDPlayerStatusWidget;

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
	
	// PlayerStatus 배치용 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD")
	TObjectPtr<UPanelWidget> PlayerStatusSlot;
	
	// PlayerStatus 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UEDPlayerStatusWidget> PlayerStatusWidgetClass;
	
	// 생성된 테스트 위젯 참조 보관용
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HUD")
	TObjectPtr<UEDPlayerStatusWidget> PlayerStatusWidgetInstance;
	
private:
	// PlayerStatus 위젯 생성용
	void CreatePlayerStatusWidget();
};
