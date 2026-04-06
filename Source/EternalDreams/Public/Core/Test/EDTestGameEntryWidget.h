// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDTestGameEntryWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

/**
 * Simple lobby test UI.
 * Players join the dedicated lobby server and mark themselves ready.
 */
UCLASS()
class ETERNALDREAMS_API UEDTestGameEntryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> NicknameInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ServerIPInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyStatusText;

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnReadyClicked();

private:
	bool TrySaveNickname();
};
