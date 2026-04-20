// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Player/EDPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Characters/Player/Component/EDCraftingInteractionComponent.h"
#include "Characters/Player/Component/EDLootInteractionComponent.h"
#include "Characters/Player/OtherActor/EDCameraActor.h"
#include "Characters/Player/OtherActor/EDCursorActor.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/HUD/EDDeathOverlayWidget.h"
#include "UI/HUD/EDRespawnZoneSelectWidget.h"
#include "UI/HUD/EDMatchResultWidget.h"
#include "UI/Types/EDUIWidgetIds.h"
#include "Misc/CoreDelegates.h"
#include "Core/EDGameMode.h"
#include "Core/EDPlayerState.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "CommonActivatableWidget.h"

AEDPlayerController::AEDPlayerController()
{
	CraftingInteractionComponent = CreateDefaultSubobject<UEDCraftingInteractionComponent>(TEXT("CraftingInteractionComponent"));
	LootInteractionComponent = CreateDefaultSubobject<UEDLootInteractionComponent>(TEXT("LootInteractionComponent"));
}


// IGenericTeamAgentInterface
void AEDPlayerController::SetGenericTeamId(const FGenericTeamId& NewTeamId)
{
	CachedTeamId = NewTeamId;
}

FGenericTeamId AEDPlayerController::GetGenericTeamId() const
{
	return CachedTeamId;
}

ETeamAttitude::Type AEDPlayerController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* OtherPawn = Cast<APawn>(&Other);
	if (!IsValid(OtherPawn))
		return ETeamAttitude::Neutral;

	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
	if (!OtherTeamAgent)
		return ETeamAttitude::Neutral;

	return CachedTeamId == OtherTeamAgent->GetGenericTeamId()
		       ? ETeamAttitude::Friendly
		       : ETeamAttitude::Hostile;
}

void AEDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 로비에서 배정된 TeamId → GenericTeamId 동기화
	if (AEDPlayerState* PS = GetPlayerState<AEDPlayerState>())
	{
		SetGenericTeamId(FGenericTeamId(PS->TeamId));
	}

	if (IsLocalController())
	{
		// Game + UI 입력 모드 설정 (로비 UIOnly → 인게임 전환)


		FInputModeGameOnly InputMode;
		/*
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
				InputMode.SetHideCursorDuringCapture(false);
				InputMode.SetWidgetToFocus(nullptr);
		 *
		 */
		SetInputMode(InputMode);
		bShowMouseCursor = true;


		//Create Component
		CursorActor = GetWorld()->SpawnActor<AEDCursorActor>(CursorActorClass);
		CameraActor = GetWorld()->SpawnActor<AEDCameraActor>(CameraActorClass);
		SetViewTargetWithBlend(CameraActor);

		if (IsValid(CameraActor))
		{
			OnCameraScroll.BindUObject(CameraActor, &AEDCameraActor::CameraZoom);
			OnCameraFocus.BindUObject(CameraActor, &AEDCameraActor::ToggleCameraFocus);
		}

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CameraInputMappingContext, 0); // Gameplay

			// UI 입력 매핑 컨텍스트 등록
			if (UIInputMappingContext)
			{
				Subsystem->AddMappingContext(UIInputMappingContext, 1);
				UE_LOG(LogTemp, Log, TEXT("EDPlayerController: UIInputMappingContext 등록을 완료했습니다. 이름 = %s"),
				       *UIInputMappingContext->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIInputMappingContext가 설정되지 않았습니다."));
			}
		}
	}

	FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject(
		this, &AEDPlayerController::HandleApplicationReactivated);
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

	if (ToggleLootInventoryAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: ToggleLootInventoryAction 바인딩을 완료했습니다. 이름 = %s"),
		       *ToggleLootInventoryAction->GetName());
		EnhancedInputComponent->BindAction(ToggleLootInventoryAction, ETriggerEvent::Started, this,
		                                   &AEDPlayerController::HandleToggleLootInventory);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: ToggleLootInventoryAction이 설정되지 않았습니다."));
	}

	if (ToggleCraftPanelAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: ToggleCraftPanelAction 바인딩을 완료했습니다. 이름 = %s"),
		       *ToggleCraftPanelAction->GetName());
		EnhancedInputComponent->BindAction(ToggleCraftPanelAction, ETriggerEvent::Started, this,
		                                   &AEDPlayerController::HandleToggleCraftPanel);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: ToggleCraftPanelAction이 설정되지 않았습니다."));
	}

	if (UIBackAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIBackAction 바인딩을 완료했습니다. 이름 = %s"),
		       *UIBackAction->GetName());
		EnhancedInputComponent->BindAction(UIBackAction, ETriggerEvent::Started, this,
		                                   &AEDPlayerController::HandleUIBack);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIBackAction이 설정되지 않았습니다."));
	}

	if (CraftItemAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: CraftItemAction 바인딩을 완료했습니다. 이름 = %s"),
		       *CraftItemAction->GetName());
		EnhancedInputComponent->BindAction(CraftItemAction, ETriggerEvent::Started, this,
		                                   &AEDPlayerController::HandleCraftItem);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: CraftItemAction이 설정되지 않았습니다."));
	}


	EnhancedInputComponent->BindAction(
		WheelAction,
		ETriggerEvent::Triggered,
		this,
		&AEDPlayerController::CameraZoom
	);
	EnhancedInputComponent->BindAction(
		KeyboardCAction,
		ETriggerEvent::Started,
		this,
		&AEDPlayerController::CameraFocus
	);
}

void AEDPlayerController::HandleToggleLootInventory()
{
	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 루팅 인벤토리 토글 입력을 처리합니다."));

	if (!LootInteractionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: LootInteractionComponent가 없어 루팅 인벤토리 입력을 처리할 수 없습니다."));
		return;
	}

	LootInteractionComponent->HandleToggleLootPanel();
}

void AEDPlayerController::HandleToggleCraftPanel()
{
	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 아이템 제작 패널 토글 입력을 처리합니다."));

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: LocalPlayer가 없어 제작 패널 입력을 처리할 수 없습니다."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIManageSubsystem이 없어 제작 패널 입력을 처리할 수 없습니다."));
		return;
	}

	UIManageSubsystem->TogglePanel(EDUIWidgetIds::Panel_ItemCrafting);
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

void AEDPlayerController::HandleCraftItem()
{
	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 제작 입력을 처리합니다."));

	if (!CraftingInteractionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: CraftingInteractionComponent가 없어 제작 입력을 처리할 수 없습니다."));
		return;
	}

	CraftingInteractionComponent->HandleCraftInput();
	OnCraftInputTriggered.Broadcast();
}

void AEDPlayerController::HandleApplicationReactivated()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: 애플리케이션 복귀 시 LocalPlayer를 찾지 못했습니다."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: 애플리케이션 복귀 시 UIManageSubsystem을 찾지 못했습니다."));
		return;
	}

	// 창 복귀 직후 즉시 포커스를 한 번 복구
	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 애플리케이션 복귀로 UI 포커스 복구를 요청합니다."));
	UIManageSubsystem->RestoreUIFocus();

	if (GetWorld())
	{
		FTimerHandle RestoreFocusTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			RestoreFocusTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [LocalPlayer]()
			{
				if (!LocalPlayer)
				{
					return;
				}

				UEDUIManageSubsystem* DelayedSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
				if (!DelayedSubsystem)
				{
					return;
				}

				// 복귀 직후에는 포커스가 아직 불안정할 수 있어 다음 틱에 한 번 더 복구
				UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 다음 틱에서 UI 포커스를 한 번 더 복구합니다."));
				DelayedSubsystem->RestoreUIFocus();
			}),
			0.0f,
			false
		);
	}
}


void AEDPlayerController::CameraZoom(const FInputActionValue& value)
{
	OnCameraScroll.ExecuteIfBound(value);
}

void AEDPlayerController::CameraFocus(const FInputActionValue& value)
{
	OnCameraFocus.ExecuteIfBound(value);
}

void AEDPlayerController::ClientOnPlayerDied_Implementation(float CountdownSeconds, bool bCanRespawn)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UEDUIManageSubsystem* UIMgr = LP->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIMgr)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIManageSubsystem을 찾을 수 없습니다."));
		return;
	}

	UCommonActivatableWidget* Panel = UIMgr->OpenPanel(EDUIWidgetIds::Panel_DeathOverlay);
	if (UEDDeathOverlayWidget* Overlay = Cast<UEDDeathOverlayWidget>(Panel))
	{
		Overlay->StartCountdown(CountdownSeconds, bCanRespawn);
	}
}

void AEDPlayerController::ClientOpenZoneSelectWidget_Implementation()
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UEDUIManageSubsystem* UIMgr = LP->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIMgr)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIManageSubsystem을 찾을 수 없습니다."));
		return;
	}

	UIMgr->OpenPanel(EDUIWidgetIds::Panel_RespawnZoneSelect);
}

void AEDPlayerController::Server_RequestRespawn_Implementation(int32 SelectedZoneId)
{
	AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	GM->HandleRespawnRequest(this, SelectedZoneId);
}

void AEDPlayerController::ClientShowMatchResult_Implementation(const TArray<int32>& TeamRankings)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	UEDUIManageSubsystem* UIMgr = LP->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIMgr)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIManageSubsystem을 찾을 수 없습니다."));
		return;
	}

	UCommonActivatableWidget* Panel = UIMgr->OpenPanel(EDUIWidgetIds::Panel_MatchResult);
	if (UEDMatchResultWidget* ResultWidget = Cast<UEDMatchResultWidget>(Panel))
	{
		int32 MyTeamId = EDTeam::None;
		if (AEDPlayerState* PS = GetPlayerState<AEDPlayerState>())
		{
			MyTeamId = PS->TeamId;
		}
		ResultWidget->SetResult(TeamRankings, MyTeamId);
	}
}
