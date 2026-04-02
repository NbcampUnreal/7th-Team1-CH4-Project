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

	UE_LOG(LogTemp, Log, TEXT("[TestGameEntry] NativeConstruct 완료. JoinButton, StartButton 바인딩됨."));
}

bool UEDTestGameEntryWidget::TrySaveNickname()
{
	const FString Nickname = NicknameInput->GetText().ToString().TrimStartAndEnd();
	if (Nickname.IsEmpty())
	{
		StatusText->SetText(FText::FromString(TEXT("닉네임을 입력해주세요.")));
		UE_LOG(LogTemp, Warning, TEXT("[TestGameEntry] TrySaveNickname 실패: 닉네임 비어있음."));
		return false;
	}

	UEDGameInstance* GI = GetGameInstance<UEDGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[TestGameEntry] TrySaveNickname 실패: GameInstance nullptr."));
		return false;
	}

	GI->LocalPlayerNickname = Nickname;
	UE_LOG(LogTemp, Log, TEXT("[TestGameEntry] 닉네임 저장: %s"), *Nickname);
	return true;
}

void UEDTestGameEntryWidget::OnJoinClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TestGameEntry] JoinButton 클릭됨."));

	if (!TrySaveNickname()) return;

	const FString IP = ServerIPInput->GetText().ToString().TrimStartAndEnd();
	if (IP.IsEmpty())
	{
		StatusText->SetText(FText::FromString(TEXT("서버 IP를 입력해주세요.")));
		UE_LOG(LogTemp, Warning, TEXT("[TestGameEntry] JoinGame 실패: IP 비어있음."));
		return;
	}

	UEDGameInstance* GI = GetGameInstance<UEDGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[TestGameEntry] JoinGame 실패: GameInstance nullptr."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[TestGameEntry] JoinGame 호출 → IP: %s"), *IP);
	StatusText->SetText(FText::FromString(FString::Printf(TEXT("%s 에 접속합니다..."), *IP)));
	GI->JoinGame(IP);
}

void UEDTestGameEntryWidget::OnStartClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[TestGameEntry] StartButton 클릭됨."));

	AEDPlayerController_Temp* PC = Cast<AEDPlayerController_Temp>(GetOwningPlayer());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[TestGameEntry] OnStartClicked 실패: PlayerController_Temp 캐스트 실패."));
		StatusText->SetText(FText::FromString(TEXT("서버에 접속된 상태가 아닙니다.")));
		return;
	}

	StatusText->SetText(FText::FromString(TEXT("게임 시작 요청 중...")));
	PC->Server_RequestStartGame();
}
