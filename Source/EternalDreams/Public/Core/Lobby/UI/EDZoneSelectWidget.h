// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDZoneSelectWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEDOnZoneSelectionChanged, int32, ZoneId);

/**
 * Reusable zone selector panel.
 * This widget only owns visuals and local selection state.
 * Lobby and respawn flows should listen to the selection event and decide what to do.
 */
UCLASS()
class ETERNALDREAMS_API UEDZoneSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ED|ZoneSelector")
	void SelectZone(int32 ZoneId);

	UFUNCTION(BlueprintCallable, Category = "ED|ZoneSelector")
	void SetSelectedZone(int32 ZoneId, bool bBroadcastSelection = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ED|ZoneSelector")
	int32 GetSelectedZoneId() const { return SelectedZoneId; }

	UFUNCTION(BlueprintCallable, Category = "ED|ZoneSelector")
	void SetZoneEnabled(int32 ZoneId, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ED|ZoneSelector")
	void SetAllZonesEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ED|ZoneSelector")
	void RefreshVisuals();

	UFUNCTION(BlueprintCallable, Category = "ED|ZoneSelector")
	void ClearSelection();

	UPROPERTY(BlueprintAssignable, Category = "ED|ZoneSelector")
	FEDOnZoneSelectionChanged OnZoneSelectionChanged;

	UFUNCTION(BlueprintImplementableEvent, Category = "ED|ZoneSelector")
	void OnZoneSelected(int32 ZoneId);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton4;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ZoneLabel;

	UPROPERTY(EditDefaultsOnly, Category = "ED|ZoneSelector|Style")
	FLinearColor SelectedColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "ED|ZoneSelector|Style")
	FLinearColor NormalColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "ED|ZoneSelector|Style")
	FLinearColor DisabledColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.6f);

private:
	void UpdateButtonVisuals() const;
	UButton* GetButtonByZoneId(int32 ZoneId) const;
	bool IsValidZoneId(int32 ZoneId) const;

	UFUNCTION() void OnZone1Clicked();
	UFUNCTION() void OnZone2Clicked();
	UFUNCTION() void OnZone3Clicked();
	UFUNCTION() void OnZone4Clicked();

	UPROPERTY(VisibleAnywhere, Category = "ED|ZoneSelector")
	int32 SelectedZoneId = 0;

	static const TCHAR* ZoneNames[4];
};
