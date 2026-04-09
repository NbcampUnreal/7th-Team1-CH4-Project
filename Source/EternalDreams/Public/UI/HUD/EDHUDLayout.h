#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Components/PanelWidget.h"
#include "UI/Types/EDUITypes.h"
#include "EDHUDLayout.generated.h"

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

	// 게임 중 열리는 패널이 올라갈 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> GameLayerSlot;

	// 메뉴 계열 패널이 올라갈 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> MenuLayerSlot;

	// 모달, 확인창, 경고창 등이 올라갈 최상위 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> ModalLayerSlot;
};
