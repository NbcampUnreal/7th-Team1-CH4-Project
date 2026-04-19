// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/UI/EDLobbyZoneSelectWidget.h"

#include "Core/EDPlayerState.h"
#include "Core/Lobby/UI/EDZoneSelectWidget.h"
#include "Core/Network/EDMatchmakingSubsystem.h"
#include "Engine/GameInstance.h"

namespace
{
	bool IsValidLobbyZoneId(const int32 ZoneId)
	{
		return ZoneId >= 1 && ZoneId <= 4;
	}
}

void UEDLobbyZoneSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ZoneSelectorPanel)
	{
		return;
	}

	ZoneSelectorPanel->OnZoneSelectionChanged.RemoveDynamic(this, &UEDLobbyZoneSelectWidget::HandleZoneSelected);
	ZoneSelectorPanel->OnZoneSelectionChanged.AddDynamic(this, &UEDLobbyZoneSelectWidget::HandleZoneSelected);
	ZoneSelectorPanel->SetAllZonesEnabled(true);

	InitializeSelection();
}

void UEDLobbyZoneSelectWidget::NativeDestruct()
{
	if (ZoneSelectorPanel)
	{
		ZoneSelectorPanel->OnZoneSelectionChanged.RemoveDynamic(this, &UEDLobbyZoneSelectWidget::HandleZoneSelected);
	}

	Super::NativeDestruct();
}

void UEDLobbyZoneSelectWidget::HandleZoneSelected(int32 ZoneId)
{
	CommitZoneSelection(ZoneId);
}

void UEDLobbyZoneSelectWidget::InitializeSelection()
{
	// IOCP 로비 흐름에서는 클라이언트가 MainMenu 맵에 머무르므로
	// Subsystem이 "로비 세션" 단위의 zone 상태를 보유한다.
	// PlayerState(DesiredZoneId)는 데디서버 인게임 리스폰용으로도 쓰이므로 함께 참조한다.
	int32 InitialZoneId = DefaultZoneId;

	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UEDMatchmakingSubsystem* Matchmaking = GI->GetSubsystem<UEDMatchmakingSubsystem>())
		{
			const int32 SessionZoneId = Matchmaking->GetSelectedZoneId();
			if (IsValidLobbyZoneId(SessionZoneId))
			{
				InitialZoneId = SessionZoneId;
			}
		}
	}

	if (const AEDPlayerState* PlayerState = GetOwningPlayerState<AEDPlayerState>())
	{
		if (IsValidLobbyZoneId(PlayerState->DesiredZoneId))
		{
			InitialZoneId = PlayerState->DesiredZoneId;
		}
	}

	if (!IsValidLobbyZoneId(InitialZoneId))
	{
		return;
	}

	ZoneSelectorPanel->SetSelectedZone(InitialZoneId, false);

	CommitZoneSelection(InitialZoneId);
}

void UEDLobbyZoneSelectWidget::CommitZoneSelection(int32 ZoneId) const
{
	if (!IsValidLobbyZoneId(ZoneId))
	{
		return;
	}

	// 인게임 리스폰 플로우에서 재사용될 때를 위해 PlayerState도 갱신해둔다.
	// 메인메뉴 맵(standalone)에서는 리플리케이트되지 않고 로컬 저장으로만 동작한다.
	if (AEDPlayerState* PlayerState = GetOwningPlayerState<AEDPlayerState>())
	{
		PlayerState->DesiredZoneId = ZoneId;
	}

	// IOCP 로비 경로: 데디서버 PlayerController RPC 대신 Subsystem으로 위임.
	// 서버 측 Zone OpCode가 추가되면 Subsystem 내부에서 패킷 송신이 이어진다.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UEDMatchmakingSubsystem* Matchmaking = GI->GetSubsystem<UEDMatchmakingSubsystem>())
		{
			Matchmaking->LobbySelectZone(ZoneId);
		}
	}
}
