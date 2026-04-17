// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDRespawnZoneSelectWidget.generated.h"

class UButton;
class UEDZoneSelectWidget;

/**
 * Respawn-specific wrapper around the shared zone selector panel.
 * Keeps a pending zone locally and submits it through the gameplay controller.
 */
UCLASS()
class ETERNALDREAMS_API UEDRespawnZoneSelectWidget : public UUserWidget
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

	UPROPERTY(VisibleAnywhere, Category = "ED|Death")
	int32 PendingZoneId = 0;

	UPROPERTY()
	TArray<int32> CachedAvailableZones;
};
