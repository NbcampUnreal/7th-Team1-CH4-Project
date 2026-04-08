// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDPauseMenuWidget.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Engine/LocalPlayer.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "Components/TextBlock.h"
#include "UI/Types/EDUIWidgetIds.h"

void UEDPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: 일시정지 메뉴 패널이 생성되었습니다."));
}

void UEDPauseMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetKeyboardFocus();

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: 일시정지 메뉴 패널이 열렸습니다."));
}

void UEDPauseMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: 일시정지 메뉴 패널이 닫혔습니다."));
}

FReply UEDPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
		if (!LocalPlayer)
		{
			UE_LOG(LogTemp, Warning, TEXT("EDPauseMenuWidget: LocalPlayer가 없어 ESC 입력을 처리할 수 없습니다."));
			return FReply::Handled();
		}

		UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
		if (!UIManageSubsystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("EDPauseMenuWidget: UIManageSubsystem이 없어 ESC 입력을 처리할 수 없습니다."));
			return FReply::Handled();
		}

		UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: ESC 입력으로 일시정지 메뉴를 닫습니다."));
		UIManageSubsystem->ClosePanel(EDUIWidgetIds::Panel_PauseMenu);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UEDPauseMenuWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);

	UE_LOG(LogTemp, Warning, TEXT("EDPauseMenuWidget: 포커스를 잃었습니다."));
}
