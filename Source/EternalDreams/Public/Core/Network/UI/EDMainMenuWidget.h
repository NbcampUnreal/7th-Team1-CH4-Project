// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDMainMenuWidget.generated.h"

class UWidgetSwitcher;
class UEditableTextBox;
class UTextBlock;
class UButton;
class UScrollBox;
class UEDMatchmakingSubsystem;
class UEDLobbyPlayerEntry;

/**
 * UEDMainMenuWidget
 *
 * 메인메뉴 루트 위젯. WidgetSwitcher로 Login/Matching/Lobby/GameStart 패널을 전환.
 * C++ 베이스 — BP에서 상속받아 레이아웃만 배치.
 */
UCLASS(Abstract)
class ETERNALDREAMS_API UEDMainMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// -------------------------------------------------------
	//  WidgetSwitcher
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> MainSwitcher;

	// -------------------------------------------------------
	//  Login Panel (Index 0)
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> LoginIdInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> LoginPwInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoginButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LoginStatusText;

	/** 로그인 패널에서 회원가입 패널로 이동하는 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> GoToRegisterButton;

	// -------------------------------------------------------
	//  Register Panel (Index 1)
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> RegisterIdInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> RegisterPwInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> RegisterNicknameInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RegisterButton;

	/** 회원가입 패널에서 로그인 패널로 돌아가는 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackToLoginButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RegisterStatusText;

	// -------------------------------------------------------
	//  Matchmaking Panel (Index 2)
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MatchmakingStatusText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MatchmakingTimerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelMatchButton;

	// -------------------------------------------------------
	//  Lobby Panel (Index 3)
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> PlayerListScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TeamAButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TeamBButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TeamCButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReadyButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MatchIdText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ReadyButtonText;

	// -------------------------------------------------------
	//  GameStart Panel (Index 4)
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameStartStatusText;

	// -------------------------------------------------------
	//  LobbyPlayerEntry 클래스 (BP에서 지정)
	// -------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "UI|Lobby")
	TSubclassOf<UEDLobbyPlayerEntry> PlayerEntryClass;

	// -------------------------------------------------------
	//  Subsystem 연결 설정
	// -------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "UI|Network")
	FString MatchServerHost = TEXT("127.0.0.1");

	UPROPERTY(EditDefaultsOnly, Category = "UI|Network")
	int32 MatchServerPort = 9000;

private:
	// -------------------------------------------------------
	//  Button Callbacks
	// -------------------------------------------------------

	UFUNCTION()
	void OnLoginButtonClicked();

	UFUNCTION()
	void OnGoToRegisterButtonClicked();

	UFUNCTION()
	void OnRegisterButtonClicked();

	UFUNCTION()
	void OnBackToLoginButtonClicked();

	UFUNCTION()
	void OnCancelMatchButtonClicked();

	UFUNCTION()
	void OnTeamAButtonClicked();

	UFUNCTION()
	void OnTeamBButtonClicked();

	UFUNCTION()
	void OnTeamCButtonClicked();

	UFUNCTION()
	void OnReadyButtonClicked();

	// -------------------------------------------------------
	//  Delegate Handlers
	// -------------------------------------------------------

	UFUNCTION()
	void HandleLoginResult(bool bSuccess, const FString& Nickname, const FString& Reason);

	UFUNCTION()
	void HandleRegisterResult(bool bSuccess, const FString& Reason);

	UFUNCTION()
	void HandleMatchQueued();

	UFUNCTION()
	void HandleMatchFound(const FString& MatchId);

	UFUNCTION()
	void HandleLobbyStateChanged(const FString& JsonState);

	UFUNCTION()
	void HandleGameStart(const FString& ServerIP, int32 ServerPort);

	UFUNCTION()
	void HandleMatchServerDisconnected();

	// -------------------------------------------------------
	//  Internal Helpers
	// -------------------------------------------------------

	void SwitchToPanel(int32 Index);
	void UpdatePlayerList(const FString& JsonState);
	UEDMatchmakingSubsystem* GetMatchmakingSubsystem() const;

	void BindSubsystemDelegates();
	void UnbindSubsystemDelegates();

	/** 매칭 경과 시간 타이머 */
	FTimerHandle MatchTimerHandle;
	float MatchStartTime = 0.f;

	UFUNCTION()
	void UpdateMatchTimer();

	bool bIsReady = false;
};
