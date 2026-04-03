// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDGameHUD.h"

#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UI/HUD/EDHUDLayout.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Types/EDUITypes.h"

void AEDGameHUD::BeginPlay()
{
	Super::BeginPlay();

	InitializeHUD();

	// HUD 초기화 끝난 뒤 패널 Open/Close 테스트
	RunPanelOpenCloseTest();
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

void AEDGameHUD::RunPanelOpenCloseTest() const
{
	// 테스트 패널 BP 없으면 테스트 하지 않음
	if (!TestPanelClass)
	{
		UE_LOG(LogTemp, Log, TEXT("EDGameHUD: TestPanelClass 미설정"));
		return;
	}

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: 테스트용 PlayerController가 없습니다."));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: 테스트용 LocalPlayer가 없습니다."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: 테스트용 UIManageSubsystem이 없습니다."));
		return;
	}

	// 테스트용 패널 클래스 등록
	UIManageSubsystem->RegisterPanelClass(TEXT("Inventory"), EEDUILayer::Game, TestPanelClass);

	// 등록 직후 패널 열기
	UIManageSubsystem->OpenPanel(TEXT("Inventory"));

	UE_LOG(LogTemp, Log, TEXT("EDGameHUD: 테스트 패널 열기 호출"));

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: World 없음"));
		return;
	}

	// 3초 뒤 닫아서 Open/Close 둘 다 확인
	FTimerHandle ClosePanelTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		ClosePanelTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [UIManageSubsystem]()
		{
			if (!UIManageSubsystem)
			{
				UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: 테스트 패널 닫기 실패"));
				return;
			}

			UIManageSubsystem->ClosePanel(TEXT("Inventory"));

			UE_LOG(LogTemp, Log, TEXT("EDGameHUD: 테스트 패널 닫기 호출"));
		}),
		3.0f,
		false
	);
}
