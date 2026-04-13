// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDGameHUD.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UI/HUD/EDHUDLayout.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Panel/EDPauseMenuWidget.h"
#include "UI/Data/EDUIRegistryDataAsset.h"
#include "Core/EDAssetManager.h"
#include "Blueprint/UserWidget.h"

void AEDGameHUD::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeHUD();
}

void AEDGameHUD::RegisterWidgetsFromRegistry(UEDUIManageSubsystem* UIManageSubsystem)
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
	
	// 로드한 소프트 경로 수집
	TArray<FSoftObjectPath> ClassPaths;
	for (const FEDUIRegistryEntry& Entry : UIRegistry->Entries)
	{
		if (!Entry.WidgetClass.IsNull())
		{
			ClassPaths.Add(Entry.WidgetClass.ToSoftObjectPath());
		}
	}
	
	// 등록할거 없으면 바로 보여줌
	if (ClassPaths.IsEmpty())
	{
		UIManageSubsystem->ShowHUD();
		return;
	}
	
	UEDAssetManager& AM = UEDAssetManager::Get();
	WidgetClassLoadHandle = AM.LoadAssetsAsync(
		ClassPaths,
		FStreamableDelegate::CreateUObject(this, &AEDGameHUD::OnWidgetClassesLoaded, UIManageSubsystem)
	);
}

void AEDGameHUD::OnWidgetClassesLoaded(UEDUIManageSubsystem* UIManageSubsystem)
{
	if (!UIManageSubsystem || !UIRegistry) { return; }
	
	for (const FEDUIRegistryEntry& Entry : UIRegistry->Entries)
	{
		UClass* LoadedClass = Entry.WidgetClass.Get();
		UE_LOG(LogTemp, Log, TEXT("[TEST] WidgetId=%s | 클래스 로드됨=%s | 클래스명=%s"),
			*Entry.WidgetId.ToString(),
			LoadedClass ? TEXT("YES") : TEXT("NO"),           // NO면 로드 실패
			LoadedClass ? *LoadedClass->GetName() : TEXT("null")
		);
		if (Entry.WidgetType == EEDUIWidgetType::HUD)
		{
			TSubclassOf<UEDHUDLayout> HUDClass = Cast<UClass>(Entry.WidgetClass.Get());
			if (!HUDClass) { continue; }
			UIManageSubsystem->SetHUDLayoutClass(HUDClass);
		}
		else if (Entry.WidgetType == EEDUIWidgetType::Panel)
		{
			TSubclassOf<UCommonActivatableWidget> PanelClass = Cast<UClass>(Entry.WidgetClass.Get());
			if (!PanelClass) { continue; }
			UIManageSubsystem->RegisterPanelClass(Entry.WidgetId, Entry.Layer, PanelClass);
		}
	}

	UIManageSubsystem->ShowHUD();
	UE_LOG(LogTemp, Log, TEXT("EDGameHUD: 비동기 위젯 클래스 로드 및 HUD 초기화 완료"));
}

void AEDGameHUD::InitializeHUD()
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
}
