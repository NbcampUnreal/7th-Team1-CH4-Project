// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDTestGameEntryWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UWidgetSwitcher;

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

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> EntrySwitcher;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ED|TestEntry")
	int32 JoinPanelIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ED|TestEntry")
	int32 LobbyPanelIndex = 1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> NicknameInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ServerIPInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReadyStatusText;

	/** Team A 플레이어 목록 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamAListText;

	/** Team B 플레이어 목록 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamBListText;

	/** Team C 플레이어 목록 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamCListText;

	/** 팀 변경 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TeamAButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TeamBButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TeamCButton;

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnTeamAClicked();

	UFUNCTION()
	void OnTeamBClicked();

	UFUNCTION()
	void OnTeamCClicked();

private:
	bool TrySaveNickname();
	bool ShouldShowLobbyPanel() const;
	void UpdateActivePanel();
	void UpdateTeamListUI();
	void RequestChangeTeam(int32 NewTeamId);

	int32 LastActivePanelIndex = INDEX_NONE;
};
