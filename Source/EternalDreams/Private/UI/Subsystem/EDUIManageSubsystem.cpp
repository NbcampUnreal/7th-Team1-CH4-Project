// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/Subsystem/EDUIManageSubsystem.h"
#include "Public/UI/HUD/EDHUDLayout.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "CommonActivatableWidget.h"

void UEDUIManageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UEDUIManageSubsystem::Deinitialize()
{
	// 서브시스템이 내려갈 때 생성했던 HUD를 먼저 정리
	CleanupHUD();

	Super::Deinitialize();
}

void UEDUIManageSubsystem::SetHUDLayoutClass(TSubclassOf<UEDHUDLayout> InHUDLayoutClass)
{
	// HUD 생성 이후, 클래스 교체로 인한 상태 꼬임 방지
	if (IsHUDCreated())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("EDUIManageSubsystem: HUD가 이미 생성되었습니다. SetHUDLayoutClass는 CreateHUD보다 먼저 호출되어야 합니다."));
		return;
	}

	HUDLayoutClass = InHUDLayoutClass;
}

void UEDUIManageSubsystem::CreateHUD()
{
	if (IsHUDCreated())
	{
		return;
	}

	CreateHUDInternal();
}

void UEDUIManageSubsystem::ShowHUD()
{
	// HUD가 아직 없으면 표시 요청 시점에 함께 생성
	if (!IsHUDCreated())
	{
		CreateHUDInternal();
	}

	if (HUDLayoutInstance)
	{
		HUDLayoutInstance->ShowLayout();
	}
}

void UEDUIManageSubsystem::HideHUD()
{
	if (!HUDLayoutInstance)
	{
		return;
	}

	HUDLayoutInstance->HideLayout();
}

UEDHUDLayout* UEDUIManageSubsystem::GetHUDLayout() const
{
	return HUDLayoutInstance;
}

void UEDUIManageSubsystem::RegisterPanelClass(FName PanelId, TSubclassOf<UCommonActivatableWidget> PanelClass)
{
	// 잘못된 등록 방지
	if (PanelId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: PanelId가 없습니다."));
		return;
	}

	if (!PanelClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: PanelClass가 없습니다."));
		return;
	}

	RegisteredPanelClasses.Add(PanelId, PanelClass);

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 새 패널 클래스 등록 완료"));
}

UCommonActivatableWidget* UEDUIManageSubsystem::OpenPanel(FName PanelId)
{
	UCommonActivatableWidget* PanelInstance = CreatePanelInstance(PanelId);
	if (!PanelInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 패널 열기 실패"));
		return nullptr;
	}

	// CommonUI 활성 상태 전환
	PanelInstance->SetVisibility(ESlateVisibility::Visible);
	PanelInstance->ActivateWidget();

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 패널 열림"));

	return PanelInstance;
}

void UEDUIManageSubsystem::ClosePanel(FName PanelId)
{
	TObjectPtr<UCommonActivatableWidget> const* FoundPanel = PanelInstances.Find(PanelId);
	if (!FoundPanel || !(*FoundPanel))
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 닫을 수 있는 패널이 없습니다."));
		return;
	}

	// 비활성화 후 숨김 처리
	(*FoundPanel)->DeactivateWidget();
	(*FoundPanel)->SetVisibility(ESlateVisibility::Collapsed);

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 패널 닫힘"));
}

void UEDUIManageSubsystem::TogglePanel(FName PanelId)
{
	if (IsPanelOpen(PanelId))
	{
		ClosePanel(PanelId);
		return;
	}

	OpenPanel(PanelId);
}

bool UEDUIManageSubsystem::IsPanelOpen(FName PanelId) const
{
	const TObjectPtr<UCommonActivatableWidget>* FoundPanel = PanelInstances.Find(PanelId);
	if (!FoundPanel || !(*FoundPanel))
	{
		return false;
	}

	return (*FoundPanel)->IsActivated();
}

bool UEDUIManageSubsystem::IsHUDCreated() const
{
	return HUDLayoutInstance != nullptr;
}

UEDHUDLayout* UEDUIManageSubsystem::CreateHUDInternal()
{
	if (!HUDLayoutClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: HUDLayoutClass가 설정되지 않았습니다."));
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: LocalPlayer가 유효하지 않습니다."));
		return nullptr;
	}

	APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: PlayerController가 유효하지 않습니다."));
		return nullptr;
	}

	HUDLayoutInstance = CreateWidget<UEDHUDLayout>(PlayerController, HUDLayoutClass);
	if (!HUDLayoutInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: HUDLayout 위젯 생성에 실패했습니다."));
		return nullptr;
	}

	HUDLayoutInstance->AddToViewport();

	return HUDLayoutInstance;
}

void UEDUIManageSubsystem::CleanupHUD()
{
	if (!HUDLayoutInstance)
	{
		return;
	}

	// 부모에서 제거한 뒤 참조를 비워 두어 중복 접근을 방지
	HUDLayoutInstance->RemoveFromParent();
	HUDLayoutInstance = nullptr;
}

UCommonActivatableWidget* UEDUIManageSubsystem::CreatePanelInstance(FName PanelId)
{
	// 이미 생성된 패널 있으면 재사용
	if (TObjectPtr<UCommonActivatableWidget>* FoundPanel = PanelInstances.Find(PanelId))
	{
		if (*FoundPanel)
		{
			return *FoundPanel;
		}
	}

	TSubclassOf<UCommonActivatableWidget>* FoundClass = RegisteredPanelClasses.Find(PanelId);
	if (!FoundClass || !(*FoundClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 등록된 패널 클래스가 없습니다."));
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: LocalPlayer가 없습니다."));
		return nullptr;
	}

	APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: PlayerController가 없습니다."));
		return nullptr;
	}

	UCommonActivatableWidget* PanelInstance = CreateWidget<UCommonActivatableWidget>(PlayerController, *FoundClass);
	if (!PanelInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 패널 생성 실패"));
		return nullptr;
	}

	PanelInstance->AddToViewport();
	PanelInstance->SetVisibility(ESlateVisibility::Collapsed);

	PanelInstances.Add(PanelId, PanelInstance);

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 패널 생성됨"));

	return PanelInstance;
}
