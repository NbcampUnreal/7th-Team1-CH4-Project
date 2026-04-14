// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Network/UI/EDMainMenuWidget.h"
#include "Core/Network/EDMatchmakingSubsystem.h"
#include "Core/Network/UI/EDLobbyPlayerEntry.h"
#include "Components/WidgetSwitcher.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Engine/World.h"

// ============================================================
//  Lifecycle
// ============================================================

void UEDMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 바인딩
	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnLoginButtonClicked);
	}
	if (CancelMatchButton)
	{
		CancelMatchButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnCancelMatchButtonClicked);
	}
	if (TeamAButton)
	{
		TeamAButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnTeamAButtonClicked);
	}
	if (TeamBButton)
	{
		TeamBButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnTeamBButtonClicked);
	}
	if (TeamCButton)
	{
		TeamCButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnTeamCButtonClicked);
	}
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnReadyButtonClicked);
	}
	if (RegisterButton)
	{
		RegisterButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnRegisterButtonClicked);
	}
	if (GoToRegisterButton)
	{
		GoToRegisterButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnGoToRegisterButtonClicked);
	}
	if (BackToLoginButton)
	{
		BackToLoginButton->OnClicked.AddDynamic(this, &UEDMainMenuWidget::OnBackToLoginButtonClicked);
	}

	// Subsystem 연결 + 델리게이트 바인딩
	if (UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem())
	{
		Subsystem->ConnectToMatchServer(MatchServerHost, MatchServerPort);
		BindSubsystemDelegates();
	}

	// 초기 화면: 로그인 패널
	SwitchToPanel(0);
}

void UEDMainMenuWidget::NativeDestruct()
{
	UnbindSubsystemDelegates();

	// 매칭 타이머 정리
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTimerHandle);
	}

	Super::NativeDestruct();
}

// ============================================================
//  Panel Switching
// ============================================================

void UEDMainMenuWidget::SwitchToPanel(int32 Index)
{
	if (MainSwitcher)
	{
		MainSwitcher->SetActiveWidgetIndex(Index);
	}
}

// ============================================================
//  Subsystem Helper
// ============================================================

UEDMatchmakingSubsystem* UEDMainMenuWidget::GetMatchmakingSubsystem() const
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		return GI->GetSubsystem<UEDMatchmakingSubsystem>();
	}
	return nullptr;
}

void UEDMainMenuWidget::BindSubsystemDelegates()
{
	UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem();
	if (!Subsystem)
	{
		return;
	}

	Subsystem->OnLoginResult.AddDynamic(this, &UEDMainMenuWidget::HandleLoginResult);
	Subsystem->OnRegisterResult.AddDynamic(this, &UEDMainMenuWidget::HandleRegisterResult);
	Subsystem->OnMatchQueued.AddDynamic(this, &UEDMainMenuWidget::HandleMatchQueued);
	Subsystem->OnMatchFound.AddDynamic(this, &UEDMainMenuWidget::HandleMatchFound);
	Subsystem->OnLobbyStateChanged.AddDynamic(this, &UEDMainMenuWidget::HandleLobbyStateChanged);
	Subsystem->OnGameStart.AddDynamic(this, &UEDMainMenuWidget::HandleGameStart);
	Subsystem->OnMatchServerDisconnected.AddDynamic(this, &UEDMainMenuWidget::HandleMatchServerDisconnected);
}

void UEDMainMenuWidget::UnbindSubsystemDelegates()
{
	UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem();
	if (!Subsystem)
	{
		return;
	}

	Subsystem->OnLoginResult.RemoveDynamic(this, &UEDMainMenuWidget::HandleLoginResult);
	Subsystem->OnRegisterResult.RemoveDynamic(this, &UEDMainMenuWidget::HandleRegisterResult);
	Subsystem->OnMatchQueued.RemoveDynamic(this, &UEDMainMenuWidget::HandleMatchQueued);
	Subsystem->OnMatchFound.RemoveDynamic(this, &UEDMainMenuWidget::HandleMatchFound);
	Subsystem->OnLobbyStateChanged.RemoveDynamic(this, &UEDMainMenuWidget::HandleLobbyStateChanged);
	Subsystem->OnGameStart.RemoveDynamic(this, &UEDMainMenuWidget::HandleGameStart);
	Subsystem->OnMatchServerDisconnected.RemoveDynamic(this, &UEDMainMenuWidget::HandleMatchServerDisconnected);
}

// ============================================================
//  Button Callbacks
// ============================================================

void UEDMainMenuWidget::OnLoginButtonClicked()
{
	UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem();
	if (!Subsystem)
	{
		return;
	}

	const FString Id = LoginIdInput ? LoginIdInput->GetText().ToString() : FString();
	const FString Pw = LoginPwInput ? LoginPwInput->GetText().ToString() : FString();

	if (Id.IsEmpty() || Pw.IsEmpty())
	{
		if (LoginStatusText)
		{
			LoginStatusText->SetText(FText::FromString(TEXT("ID와 비밀번호를 입력해주세요.")));
		}
		return;
	}

	if (LoginStatusText)
	{
		LoginStatusText->SetText(FText::FromString(TEXT("로그인 중...")));
	}

	Subsystem->Login(Id, Pw);
}

void UEDMainMenuWidget::OnGoToRegisterButtonClicked()
{
	SwitchToPanel(1);
}

void UEDMainMenuWidget::OnRegisterButtonClicked()
{
	UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem();
	if (!Subsystem)
	{
		return;
	}

	const FString Id = RegisterIdInput ? RegisterIdInput->GetText().ToString() : FString();
	const FString Pw = RegisterPwInput ? RegisterPwInput->GetText().ToString() : FString();
	const FString Nick = RegisterNicknameInput ? RegisterNicknameInput->GetText().ToString() : FString();

	if (Id.IsEmpty() || Pw.IsEmpty() || Nick.IsEmpty())
	{
		if (RegisterStatusText)
		{
			RegisterStatusText->SetText(FText::FromString(TEXT("ID, 비밀번호, 닉네임을 모두 입력해주세요.")));
		}
		return;
	}

	if (RegisterStatusText)
	{
		RegisterStatusText->SetText(FText::FromString(TEXT("회원가입 중...")));
	}

	Subsystem->Register(Id, Pw, Nick);
}

void UEDMainMenuWidget::OnBackToLoginButtonClicked()
{
	SwitchToPanel(0);
}

void UEDMainMenuWidget::OnCancelMatchButtonClicked()
{
	if (UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem())
	{
		Subsystem->CancelMatchmaking();
	}

	// 타이머 정리 후 로그인 패널로 복귀
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTimerHandle);
	}
	SwitchToPanel(0);
}

void UEDMainMenuWidget::OnTeamAButtonClicked()
{
	if (UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem())
	{
		Subsystem->LobbyChangeTeam(1);
	}
}

void UEDMainMenuWidget::OnTeamBButtonClicked()
{
	if (UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem())
	{
		Subsystem->LobbyChangeTeam(2);
	}
}

void UEDMainMenuWidget::OnTeamCButtonClicked()
{
	if (UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem())
	{
		Subsystem->LobbyChangeTeam(3);
	}
}

void UEDMainMenuWidget::OnReadyButtonClicked()
{
	if (UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem())
	{
		Subsystem->LobbySetReady();
		bIsReady = !bIsReady;

		if (ReadyButtonText)
		{
			ReadyButtonText->SetText(FText::FromString(bIsReady ? TEXT("Ready 취소") : TEXT("Ready")));
		}
	}
}

// ============================================================
//  Delegate Handlers
// ============================================================

void UEDMainMenuWidget::HandleLoginResult(bool bSuccess, const FString& InNickname, const FString& Reason)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenu: 로그인 성공 — %s"), *InNickname);

		// 로그인 성공 → 자동 매칭 요청
		if (UEDMatchmakingSubsystem* Subsystem = GetMatchmakingSubsystem())
		{
			Subsystem->RequestMatchmaking();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: 로그인 실패 — %s"), *Reason);
		if (LoginStatusText)
		{
			LoginStatusText->SetText(FText::FromString(Reason));
		}
	}
}

void UEDMainMenuWidget::HandleRegisterResult(bool bSuccess, const FString& Reason)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenu: 회원가입 성공"));
		if (RegisterStatusText)
		{
			RegisterStatusText->SetText(FText::FromString(TEXT("회원가입 성공! 로그인해주세요.")));
		}
		// 회원가입 성공 → 로그인 패널로 자동 전환
		SwitchToPanel(0);
		if (LoginStatusText)
		{
			LoginStatusText->SetText(FText::FromString(TEXT("회원가입 성공! 로그인해주세요.")));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: 회원가입 실패 — %s"), *Reason);

		FString DisplayMsg;
		if (Reason == TEXT("duplicate_id"))
		{
			DisplayMsg = TEXT("이미 존재하는 ID입니다.");
		}
		else if (Reason == TEXT("duplicate_nickname"))
		{
			DisplayMsg = TEXT("이미 존재하는 닉네임입니다.");
		}
		else if (Reason == TEXT("empty_field"))
		{
			DisplayMsg = TEXT("모든 항목을 입력해주세요.");
		}
		else
		{
			DisplayMsg = Reason;
		}

		if (RegisterStatusText)
		{
			RegisterStatusText->SetText(FText::FromString(DisplayMsg));
		}
	}
}

void UEDMainMenuWidget::HandleMatchQueued()
{
	UE_LOG(LogTemp, Log, TEXT("MainMenu: 매칭 큐 진입"));

	SwitchToPanel(2);

	if (MatchmakingStatusText)
	{
		MatchmakingStatusText->SetText(FText::FromString(TEXT("매칭 중...")));
	}

	// 경과 시간 타이머 시작
	MatchStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MatchTimerHandle,
			this,
			&UEDMainMenuWidget::UpdateMatchTimer,
			1.0f,
			true
		);
	}
}

void UEDMainMenuWidget::HandleMatchFound(const FString& MatchId)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenu: 매치 성립 — %s"), *MatchId);

	// 타이머 정리
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTimerHandle);
	}

	SwitchToPanel(3);

	if (MatchIdText)
	{
		MatchIdText->SetText(FText::FromString(FString::Printf(TEXT("Match: %s"), *MatchId)));
	}

	// 레디 상태 초기화
	bIsReady = false;
	if (ReadyButtonText)
	{
		ReadyButtonText->SetText(FText::FromString(TEXT("Ready")));
	}
}

void UEDMainMenuWidget::HandleLobbyStateChanged(const FString& JsonState)
{
	UpdatePlayerList(JsonState);
}

void UEDMainMenuWidget::HandleGameStart(const FString& ServerIP, int32 ServerPort)
{
	UE_LOG(LogTemp, Log, TEXT("MainMenu: 게임 시작 — %s:%d"), *ServerIP, ServerPort);
	SwitchToPanel(4);

	if (GameStartStatusText)
	{
		GameStartStatusText->SetText(FText::FromString(TEXT("게임 시작 중...")));
	}

	// ClientTravel은 Subsystem 내부에서 자동 처리
}

void UEDMainMenuWidget::HandleMatchServerDisconnected()
{
	UE_LOG(LogTemp, Warning, TEXT("MainMenu: 매치서버 연결 끊김"));

	// 타이머 정리
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MatchTimerHandle);
	}

	SwitchToPanel(0);

	if (LoginStatusText)
	{
		LoginStatusText->SetText(FText::FromString(TEXT("서버 연결이 끊어졌습니다.")));
	}
}

// ============================================================
//  Helpers
// ============================================================

void UEDMainMenuWidget::UpdateMatchTimer()
{
	if (!MatchmakingTimerText || !GetWorld())
	{
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - MatchStartTime;
	const int32 Minutes = FMath::FloorToInt(Elapsed / 60.f);
	const int32 Seconds = FMath::FloorToInt(FMath::Fmod(Elapsed, 60.f));

	MatchmakingTimerText->SetText(
		FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds))
	);
}

void UEDMainMenuWidget::UpdatePlayerList(const FString& JsonState)
{
	if (!PlayerListScrollBox)
	{
		return;
	}

	// JSON 파싱
	TSharedPtr<FJsonObject> RootObj;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonState);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: LOBBY_STATE JSON 파싱 실패"));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* PlayersArray = nullptr;
	if (!RootObj->TryGetArrayField(TEXT("players"), PlayersArray))
	{
		return;
	}

	// 기존 항목 제거
	PlayerListScrollBox->ClearChildren();

	for (const TSharedPtr<FJsonValue>& PlayerVal : *PlayersArray)
	{
		const TSharedPtr<FJsonObject> PlayerObj = PlayerVal->AsObject();
		if (!PlayerObj.IsValid())
		{
			continue;
		}

		const FString PlayerNickname = PlayerObj->GetStringField(TEXT("nickname"));
		const int32 TeamId = PlayerObj->GetIntegerField(TEXT("team"));
		const bool bReady = PlayerObj->GetBoolField(TEXT("ready"));

		if (!PlayerEntryClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("MainMenu: PlayerEntryClass가 설정되지 않았습니다."));
			break;
		}

		UEDLobbyPlayerEntry* Entry = CreateWidget<UEDLobbyPlayerEntry>(GetOwningPlayer(), PlayerEntryClass);
		if (Entry)
		{
			Entry->SetPlayerInfo(PlayerNickname, TeamId, bReady);
			PlayerListScrollBox->AddChild(Entry);
		}
	}
}
