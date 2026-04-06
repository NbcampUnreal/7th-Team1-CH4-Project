// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Player/EDPlayerController.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Characters/Player/CursorActor.h"
#include "Components/WidgetComponent.h"

#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"

AEDPlayerController::AEDPlayerController()
{

}

void AEDPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		CursorActor=GetWorld()->SpawnActor<ACursorActor>(CursorActorClass);
	}
}


void AEDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: SetupInputComponent가 호출되었습니다."));

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: EnhancedInputComponent를 찾을 수 없습니다."));
		return;
	}

	if (ToggleInventoryAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: ToggleInventoryAction 바인딩을 완료했습니다. 이름 = %s"),
		       *ToggleInventoryAction->GetName());
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Triggered, this,
		                                   &AEDPlayerController::HandleToggleInventory);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: ToggleInventoryAction이 설정되지 않았습니다."));
	}

	if (UIBackAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIBackAction 바인딩을 완료했습니다. 이름 = %s"),
		       *UIBackAction->GetName());
		EnhancedInputComponent->BindAction(UIBackAction, ETriggerEvent::Triggered, this,
		                                   &AEDPlayerController::HandleUIBack);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIBackAction이 설정되지 않았습니다."));
	}
}

void AEDPlayerController::HandleToggleInventory()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: LocalPlayer가 없어 인벤토리 입력을 처리할 수 없습니다."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIManageSubsystem이 없어 인벤토리 입력을 처리할 수 없습니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 인벤토리 토글 입력을 처리합니다."));
	UIManageSubsystem->TogglePanel(TEXT("Inventory"));
}

void AEDPlayerController::HandleUIBack()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: LocalPlayer가 없어 ESC 입력을 처리할 수 없습니다."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIManageSubsystem이 없어 ESC 입력을 처리할 수 없습니다."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: ESC 입력을 처리합니다."));
	UIManageSubsystem->HandleEscapeAction();
}
