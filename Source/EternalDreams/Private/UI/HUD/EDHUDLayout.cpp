// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/HUD/EDHUDLayout.h"

#include "Components/PanelWidget.h"
#include "Components/OverlaySlot.h"
#include "UI/HUD/EDToastMessageWidget.h"
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

void UEDHUDLayout::ShowToastMessage(const FText& InMessage, EEDUIMessageType InMessageType, float InDuration)
{
	if (!ToastLayerSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: ToastLayerSlot이 설정되지 않았습니다."));
		return;
	}

	if (!ToastMessageWidgetInstance)
	{
		if (!ToastMessageWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: ToastMessageWidgetClass가 설정되지 않았습니다."));
			return;
		}

		ToastMessageWidgetInstance = CreateWidget<UEDToastMessageWidget>(GetOwningPlayer(), ToastMessageWidgetClass);
		if (!ToastMessageWidgetInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: ToastMessageWidget 생성에 실패했습니다."));
			return;
		}

		if (UPanelSlot* AddedSlot = ToastLayerSlot->AddChild(ToastMessageWidgetInstance))
		{
			if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(AddedSlot))
			{
				OverlaySlot->SetHorizontalAlignment(HAlign_Center);
				OverlaySlot->SetVerticalAlignment(VAlign_Top);
			}
		}
	}

	ToastMessageWidgetInstance->ShowToastMessage(InMessage, InMessageType, InDuration);
}
