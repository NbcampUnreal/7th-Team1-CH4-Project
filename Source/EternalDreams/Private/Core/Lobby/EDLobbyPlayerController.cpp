// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/EDLobbyPlayerController.h"
#include "EternalDreams.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyGameMode.h"
#include "Core/Lobby/EDLobbyGameState.h"
#include "Core/Lobby/UI/EDZoneSelectWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AEDLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogEDCore, Warning, TEXT("[LobbyPC] BeginPlay — IsLocal: %s, NetMode: %d"),
		IsLocalController() ? TEXT("true") : TEXT("false"), static_cast<int32>(GetNetMode()));

	if (!IsLocalController()) return;

	// UI 전용 입력 모드 + 마우스 커서 표시
	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	// 로비 위젯 생성
	if (LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();

			if (UEDZoneSelectWidget* DirectZoneWidget = Cast<UEDZoneSelectWidget>(LobbyWidget))
			{
				DirectZoneWidget->OnZoneSelectionChanged.RemoveDynamic(this, &AEDLobbyPlayerController::HandleDirectZoneSelection);
				DirectZoneWidget->OnZoneSelectionChanged.AddDynamic(this, &AEDLobbyPlayerController::HandleDirectZoneSelection);

				const AEDPlayerState* EDPlayerState = GetPlayerState<AEDPlayerState>();
				const int32 InitialZoneId = (EDPlayerState && EDPlayerState->DesiredZoneId >= 1 && EDPlayerState->DesiredZoneId <= 4)
					? EDPlayerState->DesiredZoneId
					: 1;

				DirectZoneWidget->SetSelectedZone(InitialZoneId, false);

				if (!EDPlayerState || EDPlayerState->DesiredZoneId < 1 || EDPlayerState->DesiredZoneId > 4)
				{
					HandleDirectZoneSelection(InitialZoneId);
				}
			}
		}
	}
}

void AEDLobbyPlayerController::HandleDirectZoneSelection(int32 ZoneId)
{
	if (ZoneId < 1 || ZoneId > 4)
	{
		return;
	}

	if (AEDPlayerState* EDPlayerState = GetPlayerState<AEDPlayerState>())
	{
		EDPlayerState->DesiredZoneId = ZoneId;
	}

	Server_SelectZone(ZoneId);
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
