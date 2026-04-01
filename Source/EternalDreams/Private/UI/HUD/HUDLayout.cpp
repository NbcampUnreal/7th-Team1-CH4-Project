// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/HUD/HUDLayout.h"

#include "Components/PanelWidget.h"

void UHUDLayout::NativeConstruct()
{
	Super::NativeConstruct();

	// 게임 시작 직후 보이도록 설정
	SetVisibility(ESlateVisibility::Visible);
}

void UHUDLayout::NativeDestruct()
{
	Super::NativeDestruct();
}

void UHUDLayout::ShowLayout()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UHUDLayout::HideLayout()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
