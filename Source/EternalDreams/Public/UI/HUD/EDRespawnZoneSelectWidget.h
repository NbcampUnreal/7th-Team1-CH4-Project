// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDRespawnZoneSelectWidget.generated.h"

class UButton;
class UEDZoneSelectWidget;

/**
 * 부활 구역 선택 패널. UIManager의 OpenPanel(Panel_RespawnZoneSelect)로 열린다.
 * 내부 ZoneSelectorPanel(UEDZoneSelectWidget)에서 구역 선택 이벤트 수신 후,
 * ConfirmButton으로 확정하거나 bAutoSubmitOnSelection=true면 즉시 서버에 부활 요청.
 * 요청 후 스스로 ClosePanel 호출.
 */
UCLASS()
class ETERNALDREAMS_API UEDRespawnZoneSelectWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ED|Death")
	void SetAvailableZones(const TArray<int32>& AvailableZoneIds);

	UFUNCTION(BlueprintCallable, Category = "ED|Death")
	void SubmitSelectedZone();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEDZoneSelectWidget> ZoneSelectorPanel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(EditDefaultsOnly, Category = "ED|Death")
	bool bAutoSubmitOnSelection = false;

private:
	UFUNCTION()
	void HandleZoneSelected(int32 ZoneId);

	UFUNCTION()
	void HandleConfirmClicked();

	void InitializeSelection();
	void UpdateConfirmButtonState() const;
	bool IsZoneAvailable(int32 ZoneId) const;
	void CloseSelfPanel();

	UPROPERTY(VisibleAnywhere, Category = "ED|Death")
	int32 PendingZoneId = 0;

	UPROPERTY()
	TArray<int32> CachedAvailableZones;
};
