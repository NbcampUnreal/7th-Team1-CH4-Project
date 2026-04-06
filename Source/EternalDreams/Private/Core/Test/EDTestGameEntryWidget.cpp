// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Test/EDTestGameEntryWidget.h"
#include "Core/EDGameInstance.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Core/EDPlayerState.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

namespace
{
	const TCHAR* NetModeToString(const UWorld* World)
	{
		if (!World)
		{
			return TEXT("NoWorld");
		}

		switch (World->GetNetMode())
		{
		case NM_Standalone:
			return TEXT("Standalone");
		case NM_DedicatedServer:
			return TEXT("DedicatedServer");
		case NM_ListenServer:
			return TEXT("ListenServer");
		case NM_Client:
			return TEXT("Client");
		default:
			return TEXT("Unknown");
		}
	}

	bool IsConnectedLobbyClient(const UWorld* World, const APlayerController* PC)
	{
		return World
			&& PC
			&& World->GetNetMode() == NM_Client
			&& !PC->HasAuthority();
	}
}

void UEDTestGameEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!NicknameInput || !ServerIPInput || !JoinButton || !ReadyButton || !StatusText || !ReadyStatusText)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyWidget] NativeConstruct failed - one or more BindWidget references are null"));
		return;
	}

	NicknameInput->SetText(FText::FromString(TEXT("Player")));
	ServerIPInput->SetText(FText::FromString(TEXT("127.0.0.1:17777")));

	JoinButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnJoinClicked);
	ReadyButton->OnClicked.AddDynamic(this, &UEDTestGameEntryWidget::OnReadyClicked);

	ReadyStatusText->SetText(FText::FromString(TEXT("Not Ready")));

	UE_LOG(LogTemp, Log, TEXT("[LobbyWidget] NativeConstruct complete | Map=%s | NetMode=%s"),
		GetWorld() ? *GetWorld()->GetMapName() : TEXT("None"),
		NetModeToString(GetWorld()));
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
		UE_LOG(LogTemp, Error, TEXT("[LobbyWidget] TrySaveNickname failed - NicknameInput is null"));
		return false;
	}

	const FString Nickname = NicknameInput->GetText().ToString().TrimStartAndEnd();
	if (Nickname.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyWidget] TrySaveNickname failed - nickname is empty"));
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("닉네임을 입력해주세요.")));
		}
		return false;
	}

	UEDGameInstance* GI = GetGameInstance<UEDGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyWidget] TrySaveNickname failed - GameInstance is null"));
		return false;
	}

	GI->LocalPlayerNickname = Nickname;
	return true;
}

void UEDTestGameEntryWidget::OnJoinClicked()
{
	UE_LOG(LogTemp, Log, TEXT("[LobbyWidget] Join button pressed | Map=%s | NetMode=%s"),
		GetWorld() ? *GetWorld()->GetMapName() : TEXT("None"),
		NetModeToString(GetWorld()));

	if (!TrySaveNickname())
	{
		return;
	}

	if (!ServerIPInput)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyWidget] OnJoinClicked failed - ServerIPInput is null"));
		return;
	}

	const FString IP = ServerIPInput->GetText().ToString().TrimStartAndEnd();
	if (IP.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyWidget] OnJoinClicked failed - server IP is empty"));
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("서버 IP를 입력해주세요.")));
		}
		return;
	}

	UEDGameInstance* GI = GetGameInstance<UEDGameInstance>();
	if (!GI)
	{
		UE_LOG(LogTemp, Error, TEXT("[LobbyWidget] OnJoinClicked failed - GameInstance is null"));
		return;
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(FString::Printf(TEXT("%s 접속중입니다.."), *IP)));
	}

	UWorld* World = GetWorld();
	APlayerController* OwningPC = GetOwningPlayer();
	UE_LOG(LogTemp, Log, TEXT("[LobbyWidget] Join clicked | Nickname=%s | InputIP=%s | Map=%s | NetMode=%s | HasAuthority=%s"),
		*GI->LocalPlayerNickname,
		*IP,
		World ? *World->GetMapName() : TEXT("None"),
		NetModeToString(World),
		(OwningPC && OwningPC->HasAuthority()) ? TEXT("true") : TEXT("false"));

	GI->JoinGame(IP);
}

void UEDTestGameEntryWidget::OnReadyClicked()
{
	UWorld* World = GetWorld();
	AEDLobbyPlayerController* PC = Cast<AEDLobbyPlayerController>(GetOwningPlayer());
	if (!PC || !IsConnectedLobbyClient(World, PC))
	{
		UE_LOG(LogTemp, Warning, TEXT("[LobbyWidget] Ready ignored - not connected to dedicated lobby server | Map=%s | NetMode=%s | HasAuthority=%s"),
			World ? *World->GetMapName() : TEXT("None"),
			NetModeToString(World),
			(PC && PC->HasAuthority()) ? TEXT("true") : TEXT("false"));
		if (StatusText)
		{
			StatusText->SetText(FText::FromString(TEXT("서버 로비에 접속된 뒤 Ready 할 수 있습니다.")));
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[LobbyWidget] Ready clicked | PC=%s | Map=%s | NetMode=%s | HasAuthority=%s"),
		*PC->GetName(),
		World ? *World->GetMapName() : TEXT("None"),
		NetModeToString(World),
		PC->HasAuthority() ? TEXT("true") : TEXT("false"));

	PC->Server_SetReady();
}
