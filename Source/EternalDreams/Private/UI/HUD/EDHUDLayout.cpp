// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/HUD/EDHUDLayout.h"

#include "Components/PanelWidget.h"
#include "UI/Types/EDUITypes.h"

void UEDHUDLayout::NativeConstruct()
{
	Super::NativeConstruct();
}

void UEDHUDLayout::NativeDestruct()
{
	Super::NativeDestruct();
}

void UEDHUDLayout::ShowLayout()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UEDHUDLayout::HideLayout()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

UPanelWidget* UEDHUDLayout::GetLayerSlot(EEDUILayer Layer) const
{
	switch (Layer)
	{
	case EEDUILayer::Game:
		return GameLayerSlot;

	case EEDUILayer::Menu:
		return MenuLayerSlot;

	case EEDUILayer::Modal:
		return ModalLayerSlot;

	default:
		UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: 알 수 없는 UI 레이어입니다."));
		return nullptr;
	}
}
