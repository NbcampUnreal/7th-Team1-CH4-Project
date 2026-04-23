// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/HUD/EDHUDLayout.h"

#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
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

void UEDHUDLayout::SetGameLayerInputEnabled(bool bEnabled)
{
	UPanelWidget* TargetHUDLayer = PersistentHUDLayerSlot ? PersistentHUDLayerSlot.Get() : GameLayerSlot.Get();
	if (!TargetHUDLayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: 숨길 HUD 레이어가 없어 가시성을 전환할 수 없습니다."));
		return;
	}

	// Menu / Modal UI가 열려 있을 때는 항상 표시되는 HUD 레이어를 숨겨 입력 충돌을 막는다.
	TargetHUDLayer->SetVisibility(bEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("EDHUDLayout: HUD 레이어 가시성을 전환했습니다. bEnabled=%s, ChildCount=%d, Layer=%s"),
		bEnabled ? TEXT("true") : TEXT("false"),
		TargetHUDLayer->GetChildrenCount(),
		PersistentHUDLayerSlot ? TEXT("PersistentHUDLayerSlot") : TEXT("GameLayerSlot"));
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
		UE_LOG(LogTemp, Warning, TEXT("EDHUDLayout: 지원하지 않는 UI 레이어입니다."));
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
