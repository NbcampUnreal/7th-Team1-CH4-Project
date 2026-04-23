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

	// HUD 루트를 다시 표시 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowLayout();

	// HUD 루트를 숨김 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideLayout();

	// Menu / Modal UI가 열릴 때 항상 표시되는 HUD 레이어를 숨기거나 다시 표시
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetGameLayerInputEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "HUD|Layer")
	UPanelWidget* GetLayerSlot(EEDUILayer Layer) const;

	// 토스트 알림/경고 메시지를 HUD 지정 위치에 표시
	UFUNCTION(BlueprintCallable, Category = "HUD|Toast")
	void ShowToastMessage(const FText& InMessage, EEDUIMessageType InMessageType, float InDuration = 3.0f);

protected:
	// HUD 최상위 컨테이너 바인딩
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD")
	TObjectPtr<UPanelWidget> RootContainer;

	// 항상 표시되는 HUD 위젯이 올라가는 전용 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> PersistentHUDLayerSlot;

	// 게임 중 열리는 패널이 올라가는 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> GameLayerSlot;

	// 메뉴 계열 패널이 올라가는 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> MenuLayerSlot;

	// 모달, 확인창, 경고창이 올라가는 최상단 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Layer")
	TObjectPtr<UPanelWidget> ModalLayerSlot;

	// 짧은 알림/경고 메시지가 표시되는 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD|Toast")
	TObjectPtr<UPanelWidget> ToastLayerSlot;

	// 알림/경고 메시지 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Toast")
	TSubclassOf<class UEDToastMessageWidget> ToastMessageWidgetClass;

	// 실제로 생성해서 재사용하는 토스트 위젯 인스턴스
	UPROPERTY(Transient)
	TObjectPtr<class UEDToastMessageWidget> ToastMessageWidgetInstance;
};
