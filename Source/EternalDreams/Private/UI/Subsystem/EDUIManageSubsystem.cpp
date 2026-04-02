// Copyright Epic Games, Inc. All Rights Reserved.
#include "Public/UI/Subsystem/EDUIManageSubsystem.h"
#include "Public/UI/HUD/EDHUDLayout.h"

#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

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
