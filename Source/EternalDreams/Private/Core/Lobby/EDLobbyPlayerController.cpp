// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyPlayerController.h"

#include "EternalDreams.h"
#include "Blueprint/UserWidget.h"
#include "Core/EDGameInstance.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyGameMode.h"
#include "Core/Lobby/EDLobbyGameState.h"
#include "Kismet/GameplayStatics.h"

void AEDLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogEDCore, Warning, TEXT("[LobbyPC] BeginPlay — IsLocal: %s, NetMode: %d"),
		IsLocalController() ? TEXT("true") : TEXT("false"), static_cast<int32>(GetNetMode()));

	if (!IsLocalController()) return;

	if (UEDGameInstance* GI = GetGameInstance<UEDGameInstance>())
	{
		const FString Nickname = GI->LocalPlayerNickname.TrimStartAndEnd();
		if (!Nickname.IsEmpty())
		{
			Server_SetPlayerNickname(Nickname);
		}
	}

	// UI 전용 입력 모드 + 마우스 커서 표시
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	// 로비 위젯 생성 (UEDLobbyZoneSelectWidget 등 래퍼가 내부에서 존 선택 처리)
	if (LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
		}
	}
}

void AEDLobbyPlayerController::Server_SetPlayerNickname_Implementation(const FString& InNickname)
{
	AEDPlayerState* PS = GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	const FString TrimmedNickname = InNickname.TrimStartAndEnd();
	if (TrimmedNickname.IsEmpty()) return;

	PS->SetDisplayNickname(TrimmedNickname);
	PS->SetPlayerName(TrimmedNickname);
}

void AEDLobbyPlayerController::Server_SetReady_Implementation()
{
	AEDPlayerState* PS = GetPlayerState<AEDPlayerState>();
	if (!PS)
	{
		UE_LOG(LogEDCore, Error, TEXT("[LobbyPC] Server_SetReady — PlayerState null"));
		return;
	}

	PS->bReady = !PS->bReady;
	UE_LOG(LogEDCore, Warning, TEXT("[LobbyPC] Server_SetReady — Player: %s, Ready: %s"),
		*PS->GetPlayerName(), PS->bReady ? TEXT("true") : TEXT("false"));

	if (AEDLobbyGameState* LobbyGS = GetWorld()->GetGameState<AEDLobbyGameState>())
	{
		LobbyGS->UpdateTeamCounts();
	}

	AEDLobbyGameMode* GM = Cast<AEDLobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	GM->TryStartGame();
}

void AEDLobbyPlayerController::Server_ChangeTeam_Implementation(int32 NewTeamId)
{
	if (!EDTeam::IsPlayerTeam(NewTeamId)) return;

	AEDPlayerState* PS = GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	if (PS->TeamId == NewTeamId) return;

	PS->TeamId = NewTeamId;
	PS->bReady = false;

	if (AEDLobbyGameState* LobbyGS = GetWorld()->GetGameState<AEDLobbyGameState>())
	{
		LobbyGS->UpdateTeamCounts();
	}
}

void AEDLobbyPlayerController::Server_SelectZone_Implementation(int32 ZoneId)
{
	if (ZoneId < 1 || ZoneId > 4) return;
	

	AEDPlayerState* PS = GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	PS->DesiredZoneId = ZoneId;
}
