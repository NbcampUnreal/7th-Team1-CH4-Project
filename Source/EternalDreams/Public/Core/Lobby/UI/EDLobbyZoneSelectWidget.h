// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDLobbyZoneSelectWidget.generated.h"

class UEDZoneSelectWidget;

/**
 * Lobby-specific wrapper around the shared zone selector panel.
 * Commits the selected zone to the lobby controller.
 */
UCLASS()
class ETERNALDREAMS_API UEDLobbyZoneSelectWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEDZoneSelectWidget> ZoneSelectorPanel;

	UPROPERTY(EditDefaultsOnly, Category = "ED|Lobby")
	int32 DefaultZoneId = 1;

private:
	UFUNCTION()
	void HandleZoneSelected(int32 ZoneId);

	void InitializeSelection();
	void CommitZoneSelection(int32 ZoneId) const;
};
