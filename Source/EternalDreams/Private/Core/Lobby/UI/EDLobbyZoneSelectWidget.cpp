// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/UI/EDLobbyZoneSelectWidget.h"

#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Core/Lobby/UI/EDZoneSelectWidget.h"

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
	AEDPlayerState* PlayerState = GetOwningPlayerState<AEDPlayerState>();
	const int32 InitialZoneId = (PlayerState && IsValidLobbyZoneId(PlayerState->DesiredZoneId))
		? PlayerState->DesiredZoneId
		: DefaultZoneId;

	if (!IsValidLobbyZoneId(InitialZoneId))
	{
		return;
	}

	ZoneSelectorPanel->SetSelectedZone(InitialZoneId, false);

	if (!PlayerState || !IsValidLobbyZoneId(PlayerState->DesiredZoneId))
	{
		CommitZoneSelection(InitialZoneId);
	}
}

void UEDLobbyZoneSelectWidget::CommitZoneSelection(int32 ZoneId) const
{
	if (!IsValidLobbyZoneId(ZoneId))
	{
		return;
	}

	if (AEDPlayerState* PlayerState = GetOwningPlayerState<AEDPlayerState>())
	{
		PlayerState->DesiredZoneId = ZoneId;
	}

	if (AEDLobbyPlayerController* LobbyPC = Cast<AEDLobbyPlayerController>(GetOwningPlayer()))
	{
		LobbyPC->Server_SelectZone(ZoneId);
	}
}
