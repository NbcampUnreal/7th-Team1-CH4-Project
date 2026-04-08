// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Test/EDTestGameEntryWidget.h"
#include "Core/EDGameInstance.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Core/EDPlayerState.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"

void UEDTestGameEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!NicknameInput || !ServerIPInput || !JoinButton || !ReadyButton || !StatusText || !ReadyStatusText)
	{
		return;
	}

	NicknameInput->SetText(FText::FromString(TEXT("Player")));
	ServerIPInput->SetText(FText::FromString(TEXT("127.0.0.1:17777")));

	JoinButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnJoinClicked);
	ReadyButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnReadyClicked);

	if (TeamAButton) TeamAButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnTeamAClicked);
	if (TeamBButton) TeamBButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnTeamBClicked);
	if (TeamCButton) TeamCButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnTeamCClicked);

	ReadyStatusText->SetText(FText::FromString(TEXT("Not Ready")));
}

void UEDTestGameEntryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AEDPlayerState* PS = GetOwningPlayerState<AEDPlayerState>();
	if (PS && ReadyStatusText)
	{
		const FString ReadyStr = FString::Printf(TEXT("%s | %s"),
			EDTeam::GetTeamName(PS->TeamId),
			PS->bReady ? TEXT("Ready") : TEXT("Not Ready"));
		ReadyStatusText->SetText(FText::FromString(ReadyStr));
	}

	UpdateTeamListUI();
}

bool UEDTestGameEntryWidget::TrySaveNickname()
{
	if (!NicknameInput)
	{
		return false;
	}

	const FString Nickname = NicknameInput->GetText().ToString().TrimStartAndEnd();
	if (Nickname.IsEmpty())
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("닉네임을 입력해주세요.")));
		}
		return false;
	}

	UEDGameInstance* GI = GetGameInstance<UEDGameInstance>();
	if (!GI)
	{
		return false;
	}

	GI->LocalPlayerNickname = Nickname;
	return true;
}

void UEDTestGameEntryWidget::OnJoinClicked()
{
	if (!TrySaveNickname())
	{
		return;
	}

	if (!ServerIPInput)
	{
		return;
	}

	const FString IP = ServerIPInput->GetText().ToString().TrimStartAndEnd();
	if (IP.IsEmpty())
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("서버 IP를 입력해주세요.")));
		}
		return;
	}

	UEDGameInstance* GI = GetGameInstance<UEDGameInstance>();
	if (!GI)
	{
		return;
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("%s 접속중입니다.."), *IP)));
	}

	GI->JoinGame(IP);
}

void UEDTestGameEntryWidget::OnReadyClicked()
{
	AEDLobbyPlayerController* PC = Cast<AEDLobbyPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("PlayerController is not available.")));
		}
		return;
	}

	PC->Server_SetReady();
}

void UEDTestGameEntryWidget::OnTeamAClicked()
{
	RequestChangeTeam(EDTeam::TeamA);
}

void UEDTestGameEntryWidget::OnTeamBClicked()
{
	RequestChangeTeam(EDTeam::TeamB);
}

void UEDTestGameEntryWidget::OnTeamCClicked()
{
	RequestChangeTeam(EDTeam::TeamC);
}

void UEDTestGameEntryWidget::RequestChangeTeam(int32 NewTeamId)
{
	AEDLobbyPlayerController* PC = Cast<AEDLobbyPlayerController>(GetOwningPlayer());
	if (!PC) return;

	PC->Server_ChangeTeam(NewTeamId);
}

void UEDTestGameEntryWidget::UpdateTeamListUI()
{
	if (!TeamAListText || !TeamBListText || !TeamCListText) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AGameStateBase* GS = World->GetGameState();
	if (!GS) return;

	FString TeamStrings[EDTeam::PlayerTeamCount];
	for (int32 i = 0; i < EDTeam::PlayerTeamCount; ++i)
	{
		TeamStrings[i] = FString::Printf(TEXT("[ %s ]\n"), EDTeam::GetTeamName(EDTeam::PlayerTeams[i]));
	}

	for (APlayerState* BasePS : GS->PlayerArray)
	{
		AEDPlayerState* PS = Cast<AEDPlayerState>(BasePS);
		if (!PS) continue;

		const FString Name = PS->GetPlayerName();
		const FString Ready = PS->bReady ? TEXT("[Ready]") : TEXT("[Not Ready]");
		const FString Line = FString::Printf(TEXT("  %s  %s\n"), *Name, *Ready);

		for (int32 i = 0; i < EDTeam::PlayerTeamCount; ++i)
		{
			if (PS->TeamId == EDTeam::PlayerTeams[i])
			{
				TeamStrings[i] += Line;
				break;
			}
		}
	}

	TeamAListText->SetText(FText::FromString(TeamStrings[0]));
	TeamBListText->SetText(FText::FromString(TeamStrings[1]));
	TeamCListText->SetText(FText::FromString(TeamStrings[2]));
}
