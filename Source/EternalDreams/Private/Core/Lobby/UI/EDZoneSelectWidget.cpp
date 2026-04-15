// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/UI/EDZoneSelectWidget.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Components/Button.h"

void UEDZoneSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ZoneButton1) ZoneButton1->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZoneButton1Clicked);
	if (ZoneButton2) ZoneButton2->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZoneButton2Clicked);
	if (ZoneButton3) ZoneButton3->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZoneButton3Clicked);
	if (ZoneButton4) ZoneButton4->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZoneButton4Clicked);
}

void UEDZoneSelectWidget::SelectZone(int32 ZoneId)
{
	if (ZoneId < 1 || ZoneId > 4) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	AEDPlayerState* PS = PC->GetPlayerState<AEDPlayerState>();
	if (!PS) return;

	PS->DesiredZoneId = ZoneId;

	if (AEDLobbyPlayerController* LobbyPC = Cast<AEDLobbyPlayerController>(PC))
	{
		LobbyPC->Server_SelectZone(ZoneId);
	}

	OnZoneSelected(ZoneId);
}

int32 UEDZoneSelectWidget::GetSelectedZoneId() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return 0;

	const AEDPlayerState* PS = PC->GetPlayerState<AEDPlayerState>();
	return PS ? PS->DesiredZoneId : 0;
}

void UEDZoneSelectWidget::OnZoneButton1Clicked() { SelectZone(1); }
void UEDZoneSelectWidget::OnZoneButton2Clicked() { SelectZone(2); }
void UEDZoneSelectWidget::OnZoneButton3Clicked() { SelectZone(3); }
void UEDZoneSelectWidget::OnZoneButton4Clicked() { SelectZone(4); }
