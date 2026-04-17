// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Lobby/UI/EDZoneSelectWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

const TCHAR* UEDZoneSelectWidget::ZoneNames[4] = { TEXT("Zone A"), TEXT("Zone B"), TEXT("Zone C"), TEXT("Zone D") };

void UEDZoneSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ZoneButton1)
	{
		ZoneButton1->OnClicked.RemoveDynamic(this, &UEDZoneSelectWidget::OnZone1Clicked);
		ZoneButton1->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone1Clicked);
	}

	if (ZoneButton2)
	{
		ZoneButton2->OnClicked.RemoveDynamic(this, &UEDZoneSelectWidget::OnZone2Clicked);
		ZoneButton2->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone2Clicked);
	}

	if (ZoneButton3)
	{
		ZoneButton3->OnClicked.RemoveDynamic(this, &UEDZoneSelectWidget::OnZone3Clicked);
		ZoneButton3->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone3Clicked);
	}

	if (ZoneButton4)
	{
		ZoneButton4->OnClicked.RemoveDynamic(this, &UEDZoneSelectWidget::OnZone4Clicked);
		ZoneButton4->OnClicked.AddDynamic(this, &UEDZoneSelectWidget::OnZone4Clicked);
	}

	RefreshVisuals();
}

void UEDZoneSelectWidget::SelectZone(int32 ZoneId)
{
	SetSelectedZone(ZoneId, true);
}

void UEDZoneSelectWidget::SetSelectedZone(int32 ZoneId, bool bBroadcastSelection)
{
	if (!IsValidZoneId(ZoneId))
	{
		return;
	}

	if (UButton* Button = GetButtonByZoneId(ZoneId))
	{
		if (!Button->GetIsEnabled())
		{
			return;
		}
	}

	SelectedZoneId = ZoneId;
	UpdateButtonVisuals();

	if (bBroadcastSelection)
	{
		OnZoneSelectionChanged.Broadcast(ZoneId);
		OnZoneSelected(ZoneId);
	}
}

void UEDZoneSelectWidget::SetZoneEnabled(int32 ZoneId, bool bEnabled)
{
	if (UButton* Button = GetButtonByZoneId(ZoneId))
	{
		Button->SetIsEnabled(bEnabled);
		UpdateButtonVisuals();
	}
}

void UEDZoneSelectWidget::SetAllZonesEnabled(bool bEnabled)
{
	for (int32 ZoneId = 1; ZoneId <= 4; ++ZoneId)
	{
		if (UButton* Button = GetButtonByZoneId(ZoneId))
		{
			Button->SetIsEnabled(bEnabled);
		}
	}

	UpdateButtonVisuals();
}

void UEDZoneSelectWidget::RefreshVisuals()
{
	UpdateButtonVisuals();
}

void UEDZoneSelectWidget::ClearSelection()
{
	SelectedZoneId = 0;
	UpdateButtonVisuals();
}

void UEDZoneSelectWidget::UpdateButtonVisuals() const
{
	for (int32 ZoneId = 1; ZoneId <= 4; ++ZoneId)
	{
		if (UButton* Button = GetButtonByZoneId(ZoneId))
		{
			const FLinearColor TargetColor = !Button->GetIsEnabled()
				? DisabledColor
				: (ZoneId == SelectedZoneId ? SelectedColor : NormalColor);
			Button->SetBackgroundColor(TargetColor);
		}
	}

	if (!ZoneLabel)
	{
		return;
	}

	if (SelectedZoneId >= 1 && SelectedZoneId <= 4)
	{
		ZoneLabel->SetText(FText::FromString(ZoneNames[SelectedZoneId - 1]));
	}
	else
	{
		ZoneLabel->SetText(FText::FromString(TEXT("Select Zone")));
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

bool UEDZoneSelectWidget::IsValidZoneId(int32 ZoneId) const
{
	return ZoneId >= 1 && ZoneId <= 4;
}

void UEDZoneSelectWidget::OnZone1Clicked()
{
	SelectZone(1);
}

void UEDZoneSelectWidget::OnZone2Clicked()
{
	SelectZone(2);
}

void UEDZoneSelectWidget::OnZone3Clicked()
{
	SelectZone(3);
}

void UEDZoneSelectWidget::OnZone4Clicked()
{
	SelectZone(4);
}
