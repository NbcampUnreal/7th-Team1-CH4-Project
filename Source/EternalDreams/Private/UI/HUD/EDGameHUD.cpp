// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDGameHUD.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UI/HUD/EDHUDLayout.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Panel/EDPauseMenuWidget.h"
#include "UI/Data/EDUIRegistryDataAsset.h"
#include "Blueprint/UserWidget.h"

void AEDGameHUD::BeginPlay()
{
	Super::BeginPlay();

	InitializeHUD();
}

void AEDGameHUD::RegisterWidgetsFromRegistry(UEDUIManageSubsystem* UIManageSubsystem) const
{
	// Subsystem이 없으면 UI 등록 불가
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: UIManageSubsystem이 없어 Registry를 등록할 수 없습니다."));
		return;
	}

	// Registry가 비어 있으면 에디터에서 할당되지 않은 상태
	if (!UIRegistry)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: UIRegistry가 설정되지 않았습니다."));
		return;
	}

	// Registry 데이터가 올바른지 먼저 검사
	if (!UIRegistry->ValidateEntries())
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: UIRegistry 검증에 실패했습니다."));
		return;
	}

	for (const FEDUIRegistryEntry& Entry : UIRegistry->Entries)
	{
		// HUD는 HUDLayoutClass로 등록
		// Panel은 Panel 등록 흐름 사용
		if (Entry.WidgetType == EEDUIWidgetType::HUD)
		{
			// Registry에 들어있는 위젯 클래스가 실제로 HUD용 클래스인지
			// Panel용 클래스인지 확인하여 맞는 타입으로 넘김
			TSubclassOf<UEDHUDLayout> HUDClass = Cast<UClass>(Entry.WidgetClass.Get());

			// HUD 엔트리는 반드시 UEDHUDLayout 계열이어야 함
			if (!HUDClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: HUD 타입 위젯이지만 HUDLayout 클래스로 변환할 수 없습니다. WidgetId = %s"),
				       *Entry.WidgetId.ToString());
				continue;
			}

			UIManageSubsystem->SetHUDLayoutClass(HUDClass);
			UE_LOG(LogTemp, Log, TEXT("EDGameHUD: HUD Registry 등록 완료. WidgetId = %s"), *Entry.WidgetId.ToString());
			continue;
		}

		if (Entry.WidgetType == EEDUIWidgetType::Panel)
		{
			TSubclassOf<UCommonActivatableWidget> PanelClass = Cast<UClass>(Entry.WidgetClass.Get());

			// Panel 엔트리는 CommonActivatableWidget 계열이어야 패널 열기/닫기 가능
			if (!PanelClass)
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("EDGameHUD: Panel 타입 위젯이지만 CommonActivatableWidget 클래스로 변환할 수 없습니다. WidgetId = %s"),
				       *Entry.WidgetId.ToString());
				continue;
			}

			UIManageSubsystem->RegisterPanelClass(Entry.WidgetId, Entry.Layer, PanelClass);
			UE_LOG(LogTemp, Log, TEXT("EDGameHUD: Panel Registry 등록 완료. WidgetId = %s"), *Entry.WidgetId.ToString());
		}
	}
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

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDGameHUD: UIManageSubsystem이 없습니다."));
		return;
	}

	// Registry에 들어 있는 HUD/패널 정보를 먼저 등록
	RegisterWidgetsFromRegistry(UIManageSubsystem);

	// 등록된 HUDLayoutClass를 기반으로 HUD를 표시
	UIManageSubsystem->ShowHUD();

	UE_LOG(LogTemp, Log, TEXT("EDGameHUD: HUD 초기화 완료"));
}
