// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/HUD/EDHUDLayout.h"

#include "Components/PanelWidget.h"

void UEDHUDLayout::NativeConstruct()
{
	Super::NativeConstruct();

	// 게임 시작 직후 보이도록 설정
	SetVisibility(ESlateVisibility::Visible);
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
