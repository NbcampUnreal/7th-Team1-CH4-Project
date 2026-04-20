// Copyright Epic Games, Inc. All Rights Reserved.
#include "Characters/Player/Component/EDLootInteractionComponent.h"

#include "Core/EDAssetManager.h"
#include "CommonActivatableWidget.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/Message/EDUserFacingMessage.h"
#include "UI/Panel/EDInventoryPanelWidget.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Types/EDUITypes.h"
#include "UI/Types/EDUIWidgetIds.h"

UEDLootInteractionComponent::UEDLootInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UEDLootInteractionComponent::SetCurrentLootTarget(AActor* InLootTarget)
{
	if (!IsValid(InLootTarget))
	{
		CurrentLootTarget.Reset();
		return;
	}

	UEDInventoryComponent* InventoryComponent =
		UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(InLootTarget);
	if (!InventoryComponent)
	{
		CurrentLootTarget.Reset();
		UE_LOG(LogTemp, Warning, TEXT("EDLootInteractionComponent: 루팅 대상에 InventoryComponent가 없습니다. Actor=%s"),
			*InLootTarget->GetName());
		return;
	}

	CurrentLootTarget = InLootTarget;

	UE_LOG(LogTemp, Log, TEXT("EDLootInteractionComponent: 현재 루팅 대상을 설정했습니다. Actor=%s"),
		*InLootTarget->GetName());
}

void UEDLootInteractionComponent::ClearCurrentLootTarget(AActor* InLootTarget)
{
	if (InLootTarget && CurrentLootTarget.IsValid() && CurrentLootTarget.Get() != InLootTarget)
	{
		return;
	}

	const bool bHadLootTarget = CurrentLootTarget.IsValid();

	CurrentLootTarget.Reset();

	UE_LOG(LogTemp, Log, TEXT("EDLootInteractionComponent: 현재 루팅 대상을 해제했습니다."));

	if (bHadLootTarget)
	{
		CloseLootPanelIfOpen();
	}
}

AActor* UEDLootInteractionComponent::GetCurrentLootTarget() const
{
	return CurrentLootTarget.Get();
}

void UEDLootInteractionComponent::HandleToggleLootPanel()
{
	UEDUIManageSubsystem* UIManageSubsystem = GetUIManageSubsystem();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDLootInteractionComponent: UIManageSubsystem이 없어 루팅 패널을 처리할 수 없습니다."));
		return;
	}

	if (UIManageSubsystem->IsPanelOpen(EDUIWidgetIds::Panel_LootInventory))
	{
		UIManageSubsystem->ClosePanel(EDUIWidgetIds::Panel_LootInventory);
		return;
	}

	if (!CanOpenLootPanel())
	{
		UE_LOG(LogTemp, Warning, TEXT("EDLootInteractionComponent: 현재 루팅 가능한 대상이 없습니다."));
		return;
	}

	OpenLootPanelForCurrentTarget();
}

void UEDLootInteractionComponent::RequestLootTransfer(
	UEDInventoryComponent* FromInventory,
	int32 FromSlotIndex,
	int32 Quantity)
{
	PendingLootItemName = ResolveLootItemDisplayName(FromInventory, FromSlotIndex);

	if (!FromInventory)
	{
		OnLootTransferResult.Broadcast(false, EEDInventoryActionFailure::InvalidInventory);
		ShowLootTransferFailure(EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	ServerRequestLootTransfer(FromInventory, FromSlotIndex, Quantity);
}

void UEDLootInteractionComponent::ServerRequestLootTransfer_Implementation(
	UEDInventoryComponent* FromInventory,
	int32 FromSlotIndex,
	int32 Quantity)
{
	APlayerController* PlayerController = GetOwningPlayerController();
	APawn* ControlledPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
	if (!ControlledPawn)
	{
		ClientNotifyLootTransferResult(false, EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	UEDInventoryComponent* PlayerInventoryComponent =
		UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(ControlledPawn);
	if (!PlayerInventoryComponent || !FromInventory)
	{
		ClientNotifyLootTransferResult(false, EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSuccess = PlayerInventoryComponent->RequestTransferItemAutoDetailed(
		FromInventory,
		PlayerInventoryComponent,
		FromSlotIndex,
		Quantity,
		Failure);

	ClientNotifyLootTransferResult(bSuccess, Failure);
}

void UEDLootInteractionComponent::ClientNotifyLootTransferResult_Implementation(
	bool bSuccess,
	EEDInventoryActionFailure Failure)
{
	OnLootTransferResult.Broadcast(bSuccess, Failure);

	if (bSuccess)
	{
		ShowLootTransferSuccess(PendingLootItemName);
	}
	else
	{
		ShowLootTransferFailure(Failure);
	}

	PendingLootItemName = FText::GetEmpty();
}

bool UEDLootInteractionComponent::CanOpenLootPanel() const
{
	return ResolveCurrentLootInventoryComponent() != nullptr;
}

FText UEDLootInteractionComponent::ResolveLootItemDisplayName(
	UEDInventoryComponent* FromInventory,
	int32 FromSlotIndex) const
{
	if (!FromInventory || !FromInventory->InventorySlots.IsValidIndex(FromSlotIndex))
	{
		return NSLOCTEXT("LootUI", "DefaultLootItemName", "아이템");
	}

	const FEDInventorySlotData& SlotData = FromInventory->InventorySlots[FromSlotIndex];
	if (!SlotData.Item.ItemId.IsValid())
	{
		return NSLOCTEXT("LootUI", "DefaultLootItemName", "아이템");
	}

	const UEDInventoryItemDataAsset* ItemData =
		UEDAssetManager::Get().GetPrimaryAsset<UEDInventoryItemDataAsset>(SlotData.Item.ItemId);
	if (ItemData && !ItemData->DisplayName.IsEmpty())
	{
		return ItemData->DisplayName;
	}

	return FText::FromName(SlotData.Item.ItemId.PrimaryAssetName);
}

void UEDLootInteractionComponent::ShowLootTransferSuccess(const FText& ItemName) const
{
	UEDUIManageSubsystem* UIManageSubsystem = GetUIManageSubsystem();
	if (!UIManageSubsystem)
	{
		return;
	}

	const FText SafeItemName = ItemName.IsEmpty()
		? NSLOCTEXT("LootUI", "DefaultLootItemName", "아이템")
		: ItemName;

	UIManageSubsystem->ShowToastMessage(
		EDUserFacingMessage::Inventory::GetSuccessText(SafeItemName, NSLOCTEXT("LootUI", "LootAction", "획득")),
		EEDUIMessageType::Success,
		3.0f);
}

void UEDLootInteractionComponent::ShowLootTransferFailure(EEDInventoryActionFailure Failure) const
{
	UEDUIManageSubsystem* UIManageSubsystem = GetUIManageSubsystem();
	if (!UIManageSubsystem)
	{
		return;
	}

	UIManageSubsystem->ShowToastMessage(
		EDUserFacingMessage::Inventory::GetFailureText(Failure),
		EEDUIMessageType::Error,
		3.0f);
}

UEDInventoryComponent* UEDLootInteractionComponent::ResolveCurrentLootInventoryComponent() const
{
	if (!CurrentLootTarget.IsValid())
	{
		return nullptr;
	}

	return UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(CurrentLootTarget.Get());
}

void UEDLootInteractionComponent::OpenLootPanelForCurrentTarget()
{
	UEDUIManageSubsystem* UIManageSubsystem = GetUIManageSubsystem();
	if (!UIManageSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDLootInteractionComponent: UIManageSubsystem이 없어 루팅 패널을 열 수 없습니다."));
		return;
	}

	UEDInventoryComponent* LootInventoryComponent = ResolveCurrentLootInventoryComponent();
	if (!LootInventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDLootInteractionComponent: 현재 루팅 대상의 InventoryComponent를 찾지 못했습니다."));
		return;
	}

	UCommonActivatableWidget* OpenedPanel = UIManageSubsystem->OpenPanel(EDUIWidgetIds::Panel_LootInventory);
	UEDInventoryPanelWidget* InventoryPanel = Cast<UEDInventoryPanelWidget>(OpenedPanel);
	if (!InventoryPanel)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDLootInteractionComponent: InventoryPanel 캐스팅에 실패했습니다."));
		return;
	}

	InventoryPanel->SetDisplayedInventoryComponent(LootInventoryComponent);
}

void UEDLootInteractionComponent::CloseLootPanelIfOpen()
{
	UEDUIManageSubsystem* UIManageSubsystem = GetUIManageSubsystem();
	if (!UIManageSubsystem)
	{
		return;
	}

	if (UIManageSubsystem->IsPanelOpen(EDUIWidgetIds::Panel_LootInventory))
	{
		UIManageSubsystem->ClosePanel(EDUIWidgetIds::Panel_LootInventory);
	}
}

APlayerController* UEDLootInteractionComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

UEDUIManageSubsystem* UEDLootInteractionComponent::GetUIManageSubsystem() const
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
}
