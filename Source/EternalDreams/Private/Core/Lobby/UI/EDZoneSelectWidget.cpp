// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/UI/EDZoneSelectWidget.h"
#include "Core/EDPlayerState.h"
#include "Core/Lobby/EDLobbyPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

const TCHAR* UEDZoneSelectWidget::ZoneNames[4] = { TEXT("Zone A"), TEXT("Zone B"), TEXT("Zone C"), TEXT("Zone D") };

void UEDZoneSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ZoneButton1) ZoneButton1->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone1Clicked);
	if (ZoneButton2) ZoneButton2->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone2Clicked);
	if (ZoneButton3) ZoneButton3->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone3Clicked);
	if (ZoneButton4) ZoneButton4->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone4Clicked);

	SelectZone(1);
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

	UpdateButtonVisuals(ZoneId);
	OnZoneSelected(ZoneId);
}

int32 UEDZoneSelectWidget::GetSelectedZoneId() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return 0;

	const AEDPlayerState* PS = PC->GetPlayerState<AEDPlayerState>();
	return PS ? PS->DesiredZoneId : 0;
}

void UEDZoneSelectWidget::UpdateButtonVisuals(int32 SelectedZoneId)
{
	for (int32 i = 1; i <= 4; ++i)
	{
		if (UButton* Btn = GetButtonByZoneId(i))
		{
			Btn->SetBackgroundColor((i == SelectedZoneId) ? SelectedColor : NormalColor);
		}
	}

	if (ZoneLabel)
	{
		if (SelectedZoneId >= 1 && SelectedZoneId <= 4)
		{
			ZoneLabel->SetText(FText::FromString(ZoneNames[SelectedZoneId - 1]));
		}
		else
		{
			ZoneLabel->SetText(FText::FromString(TEXT("Select Zone")));
		}
	}
}

UButton* UEDZoneSelectWidget::GetButtonByZoneId(int32 ZoneId) const
{
	switch (ZoneId)
	{
	case 1: return ZoneButton1;
	case 2: return ZoneButton2;
	case 3: return ZoneButton3;
	case 4: return ZoneButton4;
	default: return nullptr;
	}
}
