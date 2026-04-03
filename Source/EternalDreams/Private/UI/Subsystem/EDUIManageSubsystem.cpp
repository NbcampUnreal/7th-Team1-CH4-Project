// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/Subsystem/EDUIManageSubsystem.h"

#include "UI/HUD/EDHUDLayout.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "CommonActivatableWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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

void UEDUIManageSubsystem::RegisterPanelClass(FName PanelId, EEDUILayer Layer,
                                              TSubclassOf<UCommonActivatableWidget> PanelClass)
{
	// 잘못된 등록 방지
	if (PanelId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 패널 ID가 비어 있습니다."));
		return;
	}

	if (!PanelClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 등록할 패널 클래스가 없습니다."));
		return;
	}

	RegisteredPanelClasses.Add(PanelId, PanelClass);
	RegisteredPanelLayers.Add(PanelId, Layer);

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 패널 클래스와 레이어 등록이 완료되었습니다. 패널 ID = %s"), *PanelId.ToString());
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

	// 패널이 열린 뒤 현재 UI 상태에 맞는 입력 모드로 갱신
	RefreshInputMode();

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

	// 패널이 열린 뒤 현재 UI 상태에 맞는 입력 모드로 갱신
	RefreshInputMode();

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

bool UEDUIManageSubsystem::HandleEscapeAction()
{
	const FName OpenPanelId = FindTopPriorityOpenPanel();
	if (!OpenPanelId.IsNone())
	{
		UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: ESC 입력으로 열린 패널을 닫습니다. 패널 ID = %s"), *OpenPanelId.ToString());
		ClosePanel(OpenPanelId);
		return true;
	}

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 열려 있는 패널이 없어 PauseMenu를 엽니다."));
	OpenPanel(TEXT("PauseMenu"));
	return true;
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

	if (!HUDLayoutInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 패널 생성을 위해 HUD를 먼저 생성합니다."));
		CreateHUDInternal();
	}

	if (!HUDLayoutInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: HUD 생성에 실패하여 패널을 만들 수 없습니다."));
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
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 패널 생성에 실패했습니다."));
		return nullptr;
	}

	if (!AttachPanelToLayer(PanelId, PanelInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 패널을 레이어에 부착하지 못했습니다."));
		return nullptr;
	}

	PanelInstance->SetVisibility(ESlateVisibility::Collapsed);
	PanelInstances.Add(PanelId, PanelInstance);

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 패널 생성 및 레이어 부착이 완료되었습니다."));
	return PanelInstance;
}

EEDUILayer UEDUIManageSubsystem::GetPanelLayer(FName PanelId) const
{
	const EEDUILayer* FoundLayer = RegisteredPanelLayers.Find(PanelId);
	if (!FoundLayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 등록된 패널 레이어가 없습니다. 패널 ID = %s"), *PanelId.ToString());
		return EEDUILayer::Game;
	}

	return *FoundLayer;
}

bool UEDUIManageSubsystem::AttachPanelToLayer(FName PanelId, UCommonActivatableWidget* PanelInstance)
{
	if (!PanelInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 레이어에 붙일 패널 인스턴스가 없습니다."));
		return false;
	}

	if (!HUDLayoutInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: HUD 레이아웃이 생성되지 않았습니다."));
		return false;
	}

	const EEDUILayer PanelLayer = GetPanelLayer(PanelId);
	UPanelWidget* LayerSlot = HUDLayoutInstance->GetLayerSlot(PanelLayer);
	if (!LayerSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 레이어 슬롯을 찾을 수 없습니다. 패널 ID = %s"), *PanelId.ToString());
		return false;
	}

	if (PanelInstance->GetParent())
	{
		UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 이미 레이어에 부착된 패널입니다. 패널 ID = %s"), *PanelId.ToString());
		return true;
	}

	LayerSlot->AddChild(PanelInstance);

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 패널을 레이어 슬롯에 부착했습니다. 패널 ID = %s"), *PanelId.ToString());
	return true;
}

FName UEDUIManageSubsystem::FindOpenPanelInLayer(EEDUILayer Layer) const
{
	for (const TPair<FName, TObjectPtr<UCommonActivatableWidget>>& PanelPair : PanelInstances)
	{
		const FName PanelId = PanelPair.Key;
		const TObjectPtr<UCommonActivatableWidget> PanelInstance = PanelPair.Value;

		if (!PanelInstance)
		{
			continue;
		}

		if (GetPanelLayer(PanelId) != Layer)
		{
			continue;
		}

		if (PanelInstance->IsActivated())
		{
			return PanelId;
		}
	}

	return NAME_None;
}

FName UEDUIManageSubsystem::FindTopPriorityOpenPanel() const
{
	FName FoundPanelId = FindOpenPanelInLayer(EEDUILayer::Modal);
	if (!FoundPanelId.IsNone())
	{
		return FoundPanelId;
	}

	FoundPanelId = FindOpenPanelInLayer(EEDUILayer::Menu);
	if (!FoundPanelId.IsNone())
	{
		return FoundPanelId;
	}

	FoundPanelId = FindOpenPanelInLayer(EEDUILayer::Game);
	if (!FoundPanelId.IsNone())
	{
		return FoundPanelId;
	}

	return NAME_None;
}

void UEDUIManageSubsystem::RefreshInputMode()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 입력 모드를 갱신할 LocalPlayer가 없습니다."));
		return;
	}

	APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDUIManageSubsystem: 입력 모드를 갱신할 PlayerController가 없습니다."));
		return;
	}

	const FName OpenModalPanel = FindOpenPanelInLayer(EEDUILayer::Modal);
	const FName OpenMenuPanel = FindOpenPanelInLayer(EEDUILayer::Menu);
	const FName OpenGamePanel = FindOpenPanelInLayer(EEDUILayer::Game);

	// Menu 또는 Modal 패널이 열려 있으면 UI 입력을 함께 받도록 유지
	if (!OpenModalPanel.IsNone() || !OpenMenuPanel.IsNone())
	{
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PlayerController, nullptr, EMouseLockMode::DoNotLock, false);
		PlayerController->bShowMouseCursor = true;

		UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 메뉴 입력 모드로 전환했습니다."));
		return;
	}

	// 현재 단계에서는 Game Layer 패널도 단축키 테스트를 위해 게임 입력을 유지
	if (!OpenGamePanel.IsNone())
	{
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
		PlayerController->bShowMouseCursor = false;

		UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 게임 레이어 패널이 열려 있어 게임 입력 모드를 유지합니다."));
		return;
	}

	UWidgetBlueprintLibrary::SetInputMode_GameOnly(PlayerController);
	PlayerController->bShowMouseCursor = false;

	UE_LOG(LogTemp, Log, TEXT("EDUIManageSubsystem: 열린 패널이 없어 게임 입력 모드로 복귀했습니다."));
}
