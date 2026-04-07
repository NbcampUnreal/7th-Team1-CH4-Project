// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/HUD/EDHUDLayout.h"

#include "Components/PanelWidget.h"
#include "UI/HUD/EDPlayerStatusWidget.h"
#include "UI/Types/EDUITypes.h"

void UEDHUDLayout::NativeConstruct()
{
	Super::NativeConstruct();

	// HUD 루트 생성 직후 테스트 위젯 붙임
	CreatePlayerStatusWidget();
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

void UEDHUDLayout::CreatePlayerStatusWidget()
{
	// 중복 생성 방지
	if (PlayerStatusWidgetInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("EDHUDLayout: PlayerStatus은 이미 존재합니다."));
		return;
	}

	if (!PlayerStatusSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: PlayerStatusSlot이 없습니다."));
		return;
	}

	if (!PlayerStatusWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: PlayerStatusWidgetClass 미설정"));
		return;
	}

	PlayerStatusWidgetInstance = CreateWidget<UEDPlayerStatusWidget>(GetOwningPlayer(), PlayerStatusWidgetClass);
	if (!PlayerStatusWidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: PlayerStatus 생성 실패"));
		return;
	}

	PlayerStatusSlot->AddChild(PlayerStatusWidgetInstance);

	UE_LOG(LogTemp, Log, TEXT("EDHUDLayout: PlayerStatus 생성 완료"));
}
