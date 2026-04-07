#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/PanelWidget.h"
#include "EDHUDLayout.generated.h"

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

	UFUNCTION(BlueprintCallable, Category = "HUD|Layer")
	UPanelWidget* GetLayerSlot(EEDUILayer Layer) const;

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

	// 인게임 패널이 올라가는 레이어 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> GameLayerSlot;

	// 메뉴 계열 패널이 올라가는 레이어 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> MenuLayerSlot;

	// 확인창, 경고창 같은 최상위 패널이 올라가는 레이어 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> ModalLayerSlot;

private:
	// PlayerStatus 위젯 생성용
	void CreatePlayerStatusWidget();
};
