// Copyright Eternal Dreams Team. All Rights Reserved.

#include "UI/HUD/EDRespawnZoneSelectWidget.h"

#include "Characters/Player/EDPlayerController.h"
#include "Components/Button.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/UI/EDZoneSelectWidget.h"

namespace
{
	bool IsValidZoneId(const int32 ZoneId)
	{
		return ZoneId >= 1 && ZoneId <= 4;
	}
}

void UEDRespawnZoneSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ZoneSelectorPanel)
	{
		ZoneSelectorPanel->OnZoneSelectionChanged.RemoveDynamic(this, &UEDRespawnZoneSelectWidget::HandleZoneSelected);
		ZoneSelectorPanel->OnZoneSelectionChanged.AddDynamic(this, &UEDRespawnZoneSelectWidget::HandleZoneSelected);
	}

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UEDRespawnZoneSelectWidget::HandleConfirmClicked);
		ConfirmButton->OnClicked.AddDynamic(this, &UEDRespawnZoneSelectWidget::HandleConfirmClicked);
	}

	InitializeSelection();
	UpdateConfirmButtonState();
}

void UEDRespawnZoneSelectWidget::NativeDestruct()
{
	if (ZoneSelectorPanel)
	{
		ZoneSelectorPanel->OnZoneSelectionChanged.RemoveDynamic(this, &UEDRespawnZoneSelectWidget::HandleZoneSelected);
	}

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.RemoveDynamic(this, &UEDRespawnZoneSelectWidget::HandleConfirmClicked);
	}

	Super::NativeDestruct();
}

void UEDRespawnZoneSelectWidget::SetAvailableZones(const TArray<int32>& AvailableZoneIds)
{
	CachedAvailableZones = AvailableZoneIds;

	if (!ZoneSelectorPanel)
	{
		return;
	}

	if (CachedAvailableZones.Num() == 0)
	{
		ZoneSelectorPanel->SetAllZonesEnabled(true);
	}
	else
	{
		ZoneSelectorPanel->SetAllZonesEnabled(false);

		for (const int32 ZoneId : CachedAvailableZones)
		{
			if (IsValidZoneId(ZoneId))
			{
				ZoneSelectorPanel->SetZoneEnabled(ZoneId, true);
			}
		}
	}

	if (!IsZoneAvailable(PendingZoneId))
	{
		PendingZoneId = 0;
		ZoneSelectorPanel->ClearSelection();
	}

	UpdateConfirmButtonState();
}

void UEDRespawnZoneSelectWidget::SubmitSelectedZone()
{
	if (!IsValidZoneId(PendingZoneId) || !IsZoneAvailable(PendingZoneId))
	{
		return;
	}

	if (AEDPlayerController* PlayerController = Cast<AEDPlayerController>(GetOwningPlayer()))
	{
		PlayerController->Server_RequestRespawn(PendingZoneId);
	}
}

void UEDRespawnZoneSelectWidget::HandleZoneSelected(int32 ZoneId)
{
	PendingZoneId = ZoneId;
	UpdateConfirmButtonState();

	if (bAutoSubmitOnSelection)
	{
		SubmitSelectedZone();
	}
}

void UEDRespawnZoneSelectWidget::HandleConfirmClicked()
{
	SubmitSelectedZone();
}

void UEDRespawnZoneSelectWidget::InitializeSelection()
{
	if (!ZoneSelectorPanel)
	{
		return;
	}

	ZoneSelectorPanel->SetAllZonesEnabled(true);

	if (AEDPlayerState* PlayerState = GetOwningPlayerState<AEDPlayerState>())
	{
		if (IsValidZoneId(PlayerState->DesiredZoneId))
		{
			PendingZoneId = PlayerState->DesiredZoneId;
			ZoneSelectorPanel->SetSelectedZone(PendingZoneId, false);
		}
	}
}

void UEDRespawnZoneSelectWidget::UpdateConfirmButtonState() const
{
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(IsValidZoneId(PendingZoneId) && IsZoneAvailable(PendingZoneId));
	}
}

bool UEDRespawnZoneSelectWidget::IsZoneAvailable(int32 ZoneId) const
{
	if (!IsValidZoneId(ZoneId))
	{
		return false;
	}

	return CachedAvailableZones.Num() == 0 || CachedAvailableZones.Contains(ZoneId);
}
