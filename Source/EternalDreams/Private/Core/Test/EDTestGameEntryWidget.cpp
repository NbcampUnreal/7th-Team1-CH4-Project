// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Test/EDTestGameEntryWidget.h"
#include "Core/EDGameInstance.h"
#include "Core/EDPlayerController_Temp.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UEDTestGameEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 로컬 테스트 기본값
	NicknameInput->SetText(FText::FromString(TEXT("Player")));
	ServerIPInput->SetText(FText::FromString(TEXT("127.0.0.1:17777")));

	JoinButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnJoinClicked);
	StartButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnStartClicked);

}

bool UEDTestGameEntryWidget::TrySaveNickname()
{
	const FString Nickname = NicknameInput->GetText().ToString().TrimStartAndEnd();
	if (Nickname.IsEmpty())
	{
		StatusText->SetText(FText::FromString(TEXT("닉네임을 입력해주세요.")));
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
	if (!TrySaveNickname()) return;

	const FString IP = ServerIPInput->GetText().ToString().TrimStartAndEnd();
	if (IP.IsEmpty())
	{
		StatusText->SetText(FText::FromString(TEXT("서버 IP를 입력해주세요.")));
		return;
	}

	UEDGameInstance* GI = GetGameInstance<UEDGameInstance>();
	if (!GI)
	{
		return;
	}

	StatusText->SetText(FText::FromString(FString::Printf(TEXT("%s 에 접속합니다..."), *IP)));
	GI->JoinGame(IP);
}

void UEDTestGameEntryWidget::OnStartClicked()
{
	AEDPlayerController_Temp* PC = Cast<AEDPlayerController_Temp>(GetOwningPlayer());
	if (!PC)
	{
		StatusText->SetText(FText::FromString(TEXT("서버에 접속된 상태가 아닙니다.")));
		return;
	}

	StatusText->SetText(FText::FromString(TEXT("게임 시작 요청 중...")));
	PC->Server_RequestStartGame();
}
