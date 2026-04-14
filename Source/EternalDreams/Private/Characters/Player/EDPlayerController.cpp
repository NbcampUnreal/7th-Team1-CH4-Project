// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Player/EDPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Characters/Player/OtherActor/EDCameraActor.h"
#include "Characters/Player/OtherActor/EDCursorActor.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "Misc/CoreDelegates.h"
#include "Core/EDGameMode.h"
#include "Core/EDPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "InputMappingContext.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "UI/Panel/EDInventoryPanelWidget.h"
#include "UI/Types/EDUIWidgetIds.h"

AEDPlayerController::AEDPlayerController()
{
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

	if (ToggleInventoryAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: ToggleInventoryAction 바인딩을 완료했습니다. 이름 = %s"),
		       *ToggleInventoryAction->GetName());
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this,
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
		EnhancedInputComponent->BindAction(UIBackAction, ETriggerEvent::Started, this,
		                                   &AEDPlayerController::HandleUIBack);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIBackAction이 설정되지 않았습니다."));
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

void AEDPlayerController::SetCurrentLootTarget(AActor* InLootTarget)
{
	// 유효하지 않은 대상이면 현재 루팅 대상 해제
	if (!IsValid(InLootTarget))
	{
		CurrentLootTarget.Reset();
		return;
	}

	// 인벤토리 컴포넌트를 제공하지 않는 대상은 루팅 대상으로 취급하지 않음
	UEDInventoryComponent* InventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(
		InLootTarget);
	if (!InventoryComponent)
	{
		CurrentLootTarget.Reset();
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: 루팅 대상에 InventoryComponent가 없습니다. Actor=%s"),
		       *InLootTarget->GetName());
		return;
	}

	CurrentLootTarget = InLootTarget;

	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 현재 루팅 대상을 설정했습니다. Actor=%s"),
	       *InLootTarget->GetName());
}

void AEDPlayerController::ClearCurrentLootTarget(AActor* InLootTarget)
{
	// 특정 대상만 해제하고 싶을 때, 현재 대상과 일치하는 경우에만 비움
	if (InLootTarget && CurrentLootTarget.IsValid() && CurrentLootTarget.Get() != InLootTarget)
	{
		return;
	}

	const bool bHadLootTarget = CurrentLootTarget.IsValid();

	CurrentLootTarget.Reset();

	UE_LOG(LogTemp, Log, TEXT("EDPlayerController: 현재 루팅 대상을 해제했습니다."));

	// 루팅 대상이 사라졌으면 열려 있는 루팅 패널도 함께 닫음.
	if (bHadLootTarget)
	{
		CloseLootPanelIfOpen();
	}
}

AActor* AEDPlayerController::GetCurrentLootTarget() const
{
	return CurrentLootTarget.Get();
}

void AEDPlayerController::SetPendingLootPanelResult(bool bInSuccess, EEDInventoryActionFailure InFailure)
{
	bHasPendingLootPanelResult = true;
	bPendingLootTransferSuccess = bInSuccess;
	PendingLootTransferFailure = InFailure;
}

bool AEDPlayerController::ConsumePendingLootPanelResult(EEDInventoryActionFailure& OutFailure)
{
	if (!bHasPendingLootPanelResult)
	{
		return false;
	}

	bHasPendingLootPanelResult = false;
	OutFailure = PendingLootTransferFailure;
	return bPendingLootTransferSuccess;
}

void AEDPlayerController::Client_NotifyLootTransferResult_Implementation(bool bSuccess,
                                                                         EEDInventoryActionFailure Failure)
{
	SetPendingLootPanelResult(bSuccess, Failure);
}

void AEDPlayerController::Server_RequestLootTransfer_Implementation(
	UEDInventoryComponent* FromInventory,
	int32 FromSlotIndex,
	int32 Quantity)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		Client_NotifyLootTransferResult(false, EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	UEDInventoryComponent* PlayerInventoryComponent =
		UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(ControlledPawn);

	if (!PlayerInventoryComponent || !FromInventory)
	{
		Client_NotifyLootTransferResult(false, EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSuccess = PlayerInventoryComponent->RequestTransferItemAutoDetailed(
		FromInventory,
		PlayerInventoryComponent,
		FromSlotIndex,
		Quantity,
		Failure
	);

	Client_NotifyLootTransferResult(bSuccess, Failure);
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

	if (UIManageSubsystem->IsPanelOpen(EDUIWidgetIds::Panel_Inventory))
	{
		UIManageSubsystem->ClosePanel(EDUIWidgetIds::Panel_Inventory);
		return;
	}

	if (!CanOpenLootPanel())
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: 현재 루팅 가능한 대상이 없습니다."));
		return;
	}

	OpenLootPanelForCurrentTarget();
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

bool AEDPlayerController::CanOpenLootPanel() const
{
	return ResolveCurrentLootInventoryComponent() != nullptr;
}

UEDInventoryComponent* AEDPlayerController::ResolveCurrentLootInventoryComponent() const
{
	if (!CurrentLootTarget.IsValid())
	{
		return nullptr;
	}

	return UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(CurrentLootTarget.Get());
}

void AEDPlayerController::OpenLootPanelForCurrentTarget()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: LocalPlayer가 없어 루팅 패널을 열 수 없습니다."));
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: UIManageSubsystem이 없어 루팅 패널을 열 수 없습니다."));
		return;
	}

	UEDInventoryComponent* LootInventoryComponent = ResolveCurrentLootInventoryComponent();
	if (!LootInventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: 현재 루팅 대상의 InventoryComponent를 찾지 못했습니다."));
		return;
	}

	UCommonActivatableWidget* OpenedPanel = UIManageSubsystem->OpenPanel(EDUIWidgetIds::Panel_Inventory);
	UEDInventoryPanelWidget* InventoryPanel = Cast<UEDInventoryPanelWidget>(OpenedPanel);
	if (!InventoryPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDPlayerController: InventoryPanel 캐스팅에 실패했습니다."));
		return;
	}

	InventoryPanel->SetDisplayedInventoryComponent(LootInventoryComponent);
}

void AEDPlayerController::CloseLootPanelIfOpen()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		return;
	}

	if (UIManageSubsystem->IsPanelOpen(EDUIWidgetIds::Panel_Inventory))
	{
		UIManageSubsystem->ClosePanel(EDUIWidgetIds::Panel_Inventory);
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

void AEDPlayerController::Server_RequestStartPhaseSequence_Implementation()
{
	AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	GM->StartPhaseSequence();
}

void AEDPlayerController::Server_RequestSkipPhase_Implementation()
{
	AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM) return;

	GM->SkipToNextPhase();
}
