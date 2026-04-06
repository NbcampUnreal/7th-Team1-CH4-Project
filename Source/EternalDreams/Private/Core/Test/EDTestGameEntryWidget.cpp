// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Test/EDTestGameEntryWidget.h"
#include "Core/EDGameInstance.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Core/EDPlayerState.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

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

	ReadyStatusText->SetText(FText::FromString(TEXT("Not Ready")));
}

void UEDTestGameEntryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AEDPlayerState* PS = GetOwningPlayerState<AEDPlayerState>();
	if (PS && ReadyStatusText)
	{
		const FString ReadyStr = FString::Printf(TEXT("Team %d | %s"),
			PS->TeamId,
			PS->bReady ? TEXT("Ready") : TEXT("Not Ready"));
		ReadyStatusText->SetText(FText::FromString(ReadyStr));
	}
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
