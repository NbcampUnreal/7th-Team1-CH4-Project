// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDPauseMenuWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Input/Reply.h"
#include "Input/UIActionBindingHandle.h"
#include "InputCoreTypes.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Types/EDUIWidgetIds.h"

void UEDPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &UEDPauseMenuWidget::HandleResumeButtonClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UEDPauseMenuWidget::HandleQuitButtonClicked);
	}

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: Pause menu constructed."));
}

void UEDPauseMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (ResumeButton)
	{
		ResumeButton->SetKeyboardFocus();
	}
	else
	{
		SetKeyboardFocus();
	}

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: Pause menu activated."));
}

void UEDPauseMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: Pause menu deactivated."));
}

FReply UEDPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ClosePauseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UEDPauseMenuWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnFocusLost(InFocusEvent);

	UE_LOG(LogTemp, Warning, TEXT("EDPauseMenuWidget: Focus lost."));
}

TOptional<FUIInputConfig> UEDPauseMenuWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, EMouseLockMode::DoNotLock, false);
}

void UEDPauseMenuWidget::HandleResumeButtonClicked()
{
	ClosePauseMenu();
}

void UEDPauseMenuWidget::HandleQuitButtonClicked()
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPauseMenuWidget: OwningPlayer is null."));
		return;
	}

	ClosePauseMenu();
	OwningPlayer->ConsoleCommand(TEXT("disconnect"));
}

void UEDPauseMenuWidget::ClosePauseMenu() const
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPauseMenuWidget: LocalPlayer is null."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPauseMenuWidget: UIManageSubsystem is null."));
		return;
	}

	UIManageSubsystem->ClosePanel(EDUIWidgetIds::Panel_PauseMenu);

	if (UCommonUIActionRouterBase* ActionRouter = LocalPlayer->GetSubsystem<UCommonUIActionRouterBase>())
	{
		ActionRouter->SetActiveUIInputConfig(
			FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown, EMouseLockMode::DoNotLock, false),
			this);
		ActionRouter->FlushInput();
	}

	UWidgetBlueprintLibrary::SetFocusToGameViewport();
}
