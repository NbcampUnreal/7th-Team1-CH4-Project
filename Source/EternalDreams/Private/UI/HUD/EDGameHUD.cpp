// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDGameHUD.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UI/HUD/EDHUDLayout.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Types/EDUITypes.h"
#include "UI/Panel/EDInventoryPanelWidget.h"
#include "UI/Panel/EDPauseMenuWidget.h"

void AEDGameHUD::BeginPlay()
{
	Super::BeginPlay();

	InitializeHUD();

	RunInventoryPanelOpenCloseTest();
}

void AEDGameHUD::InitializeHUD() const
{
	// 로컬 컨트롤러만 UI 생성 대상
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Log, TEXT("EDGameHUD: 로컬 컨트롤러가 아닙니다."));
		return;
	}

	// LocalPlayer 기준으로 Subsystem 접근
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: LocalPlayer가 없습니다."));
		return;
	}

	if (!HUDLayoutClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: HUDLayoutClass 미설정"));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: UIManageSubsystem이 없습니다."));
		return;
	}

	// HUD 클래스 먼저 주입
	UIManageSubsystem->SetHUDLayoutClass(HUDLayoutClass);

	// HUD 없으면 생성, 있으면 표시
	UIManageSubsystem->ShowHUD();

	UE_LOG(LogTemp, Log, TEXT("EDGameHUD: HUD 초기화 완료"));
}

void AEDGameHUD::RunInventoryPanelOpenCloseTest() const
{
	if (!InventoryPanelClass)
	{
		UE_LOG(LogTemp, Log, TEXT("EDGameHUD: InventoryPanelClass가 설정되지 않았습니다."));
		return;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: 인벤토리 테스트용 PlayerController가 없습니다."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: 인벤토리 테스트용 LocalPlayer가 없습니다."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: 인벤토리 테스트용 UIManageSubsystem이 없습니다."));
		return;
	}

	UIManageSubsystem->RegisterPanelClass(TEXT("Inventory"), EEDUILayer::Game, InventoryPanelClass);
	
	if (PauseMenuPanelClass)
	{
		UIManageSubsystem->RegisterPanelClass(TEXT("PauseMenu"), EEDUILayer::Menu, PauseMenuPanelClass);
		UE_LOG(LogTemp, Log, TEXT("EDGameHUD: PauseMenu 패널을 Menu 레이어에 등록했습니다."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: PauseMenuPanelClass가 설정되지 않았습니다."));
	}
}
