// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDInventoryQuickBarWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/Panel/EDEquipmentSlotWidget.h"
#include "UI/HUD/EDQuickBarSlotWidget.h"
#include "Core/EDAssetManager.h"
#include "Core/EDGameDataSubsystem.h"

#include "Components/TextBlock.h"
#include "UI/Panel/EDInventoryQuantityPopupWidget.h"

void UEDInventoryQuickBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeInventoryComponent();
	CreateQuickSlotWidgets();
	BindInventoryChanged();
	// PreloadInventoryAssets();
	
	UEDGameDataSubsystem* DS = UEDGameDataSubsystem::Get(this);
	if (!DS) return;
	if (DS->IsDataReady())
	{
		// 이미 로드 완료 → 다음 틱에 바로 실행
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UEDInventoryQuickBarWidget::PreloadInventoryAssets)
		);
	}
	else
	{
		// 아직 로드 중 → 완료 콜백에서 실행
		DS->OnAllDataLoaded.AddDynamic(
			this, &UEDInventoryQuickBarWidget::PreloadInventoryAssets);
	}
	
	
	if (QuantityPopupWidget)
	{
		QuantityPopupWidget->OnQuantityConfirmed.AddUObject(
			this, &UEDInventoryQuickBarWidget::HandleQuantityPopupConfirmed);
		QuantityPopupWidget->OnQuantityCanceled.AddUObject(
			this, &UEDInventoryQuickBarWidget::HandleQuantityPopupCanceled);
		QuantityPopupWidget->ResetPopupState();
		UE_LOG(LogTemp, Warning, TEXT("QuickBar: QuantityPopupWidget 바인딩 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("QuickBar: QuantityPopupWidget이 설정되지 않았습니다."));
	}
}

void UEDInventoryQuickBarWidget::NativeDestruct()
{
	UnbindInventoryChanged();
	
	if (UEDGameDataSubsystem* DS = UEDGameDataSubsystem::Get(this))
	{
		DS->OnAllDataLoaded.RemoveDynamic(this, &UEDInventoryQuickBarWidget::PreloadInventoryAssets);
	}
	
	if (PreloadHandle.IsValid())
	{
		PreloadHandle->ReleaseHandle();
		PreloadHandle.Reset();
	}

	Super::NativeDestruct();
}

void UEDInventoryQuickBarWidget::InitializeInventoryComponent()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryQuickBarWidget: OwningPlayerPawn을 찾을 수 없습니다."));
		return;
	}

	InventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(OwningPawn);
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryQuickBarWidget: InventoryComponent를 찾을 수 없습니다."));
	}
}

void UEDInventoryQuickBarWidget::BindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UEDInventoryQuickBarWidget::HandleInventoryChanged);
}

void UEDInventoryQuickBarWidget::UnbindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UEDInventoryQuickBarWidget::HandleInventoryChanged);
}

void UEDInventoryQuickBarWidget::CreateQuickSlotWidgets()
{
	if (!QuickSlotGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryQuickBarWidget: QuickSlotGrid가 바인딩되지 않았습니다."));
		return;
	}

	if (!QuickSlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryQuickBarWidget: QuickSlotWidgetClass가 설정되지 않았습니다."));
		return;
	}

	QuickSlotGrid->ClearChildren();
	QuickSlotWidgets.Reset();

	for (int32 SlotIndex = 0; SlotIndex < QuickSlotCount; ++SlotIndex)
	{
		UEDQuickBarSlotWidget* SlotWidget = CreateWidget<UEDQuickBarSlotWidget>(this, QuickSlotWidgetClass);
		if (!SlotWidget)
		{
			continue;
		}

		QuickSlotWidgets.Add(SlotWidget);

		const int32 Row = QuickSlotsPerRow > 0 ? SlotIndex / QuickSlotsPerRow : 0;
		const int32 Column = QuickSlotsPerRow > 0 ? SlotIndex % QuickSlotsPerRow : SlotIndex;

		if (UUniformGridSlot* GridSlot = QuickSlotGrid->AddChildToUniformGrid(SlotWidget, Row, Column))
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}

		// 슬롯 인덱스를 설정하고 클릭 이벤트를 퀵바로 연결한다.
		SlotWidget->SetSlotIndex(SlotIndex);
		SlotWidget->OnQuickBarSlotClicked.AddUObject(this, &UEDInventoryQuickBarWidget::HandleQuickSlotClicked);
		SlotWidget->OnQuickBarSlotDoubleClicked.AddUObject(
			this, &UEDInventoryQuickBarWidget::HandleQuickSlotDoubleClicked);
		SlotWidget->OnQuickBarSlotDroppedOnSlot.AddUObject(
			this, &UEDInventoryQuickBarWidget::HandleQuickSlotDroppedOnSlot);
		SlotWidget->OnQuickBarSlotDroppedOutside.AddUObject(
			this, &UEDInventoryQuickBarWidget::HandleQuickSlotDroppedOutside);
	}
}

void UEDInventoryQuickBarWidget::RefreshQuickSlots()
{
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UEDInventoryQuickBarWidget - RefreshQuickSlots] InventoryComponent nullptr"));
		return;
	}

	for (int32 Index = 0; Index < QuickSlotWidgets.Num(); ++Index)
	{
		UEDQuickBarSlotWidget* SlotWidget = QuickSlotWidgets[Index];
		if (!SlotWidget)
		{
			continue;
		}

		FEDInventorySlotData SlotData;
		if (!TryGetQuickSlotData(Index, SlotData) || SlotData.IsEmpty())
		{
			continue;
		}
		// 기존 두번 호출하던 함수를 하나로 통합하고 비동기로 변경
		const UEDInventoryItemDataAsset* ItemData = ResolveItemData(SlotData.Item.ItemId);
		const FText ItemName = (ItemData && !ItemData->DisplayName.IsEmpty())
			? ItemData->DisplayName
			: FText::FromName(SlotData.Item.ItemId.PrimaryAssetName);
		const EEDItemRarity ItemRarity = ItemData ? ItemData->Rarity : EEDItemRarity::Normal;
		
		SlotWidget->SetItemState(ItemName, SlotData.Item.Quantity, ItemRarity);
	}
}

void UEDInventoryQuickBarWidget::RefreshEquipmentSlots()
{
	if (!InventoryComponent)
	{
		return;
	}

	if (WeaponSlotWidget)
	{
		if (InventoryComponent->WeaponSlot.EquippedItem.IsValid())
		{
			const UEDInventoryItemDataAsset* Data =
				ResolveItemData(InventoryComponent->WeaponSlot.EquippedItem.ItemId);
			const FText ItemName = (Data && !Data->DisplayName.IsEmpty())
				? Data->DisplayName
				: FText::FromName(InventoryComponent->WeaponSlot.EquippedItem.ItemId.PrimaryAssetName);
			const EEDItemRarity Rarity = Data ? Data->Rarity : EEDItemRarity::Normal;
			WeaponSlotWidget->SetItemState(FText::FromString(TEXT("Weapon")), ItemName, Rarity);
		}
		else
		{
			WeaponSlotWidget->SetEmptyState(FText::FromString(TEXT("Weapon")));
		}
	}

	if (TopArmorSlotWidget)
	{
		if (InventoryComponent->TopArmorSlot.EquippedItem.IsValid())
		{
			const UEDInventoryItemDataAsset* Data =
				ResolveItemData(InventoryComponent->TopArmorSlot.EquippedItem.ItemId);
			const FText ItemName = (Data && !Data->DisplayName.IsEmpty())
				? Data->DisplayName
				: FText::FromName(InventoryComponent->TopArmorSlot.EquippedItem.ItemId.PrimaryAssetName);
			const EEDItemRarity Rarity = Data ? Data->Rarity : EEDItemRarity::Normal;
			TopArmorSlotWidget->SetItemState(FText::FromString(TEXT("Top Armor")), ItemName, Rarity);
		}
		else
		{
			TopArmorSlotWidget->SetEmptyState(FText::FromString(TEXT("Top Armor")));
		}
	}

	if (BottomArmorSlotWidget)
	{
		if (InventoryComponent->BottomArmorSlot.EquippedItem.IsValid())
		{
			const UEDInventoryItemDataAsset* Data =
				ResolveItemData(InventoryComponent->BottomArmorSlot.EquippedItem.ItemId);
			const FText ItemName = (Data && !Data->DisplayName.IsEmpty())
				? Data->DisplayName
				: FText::FromName(InventoryComponent->BottomArmorSlot.EquippedItem.ItemId.PrimaryAssetName);
			const EEDItemRarity Rarity = Data ? Data->Rarity : EEDItemRarity::Normal;
			BottomArmorSlotWidget->SetItemState(FText::FromString(TEXT("Bottom Armor")), ItemName, Rarity);
		}
		else
		{
			BottomArmorSlotWidget->SetEmptyState(FText::FromString(TEXT("Bottom Armor")));
		}
	}
}

void UEDInventoryQuickBarWidget::HandleInventoryChanged()
{
	// 같은 프레임에 중복요청 들어왔을때 다음 틱에 한번만 실행
	if (bRefreshPending)
	{
		return;
	}
	bRefreshPending = true;
	GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &UEDInventoryQuickBarWidget::DoRefresh)
	);
}

FText UEDInventoryQuickBarWidget::ResolveItemDisplayName(const FPrimaryAssetId& ItemId) const
{
	if (!ItemId.IsValid())
	{
		return FText::GetEmpty();
	}

	UObject* ItemObject = UAssetManager::Get().GetPrimaryAssetObject(ItemId);
	if (!ItemObject)
	{
		const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
		if (AssetPath.IsValid())
		{
			ItemObject = AssetPath.TryLoad();
		}
	}

	const UEDInventoryItemDataAsset* ItemData = Cast<UEDInventoryItemDataAsset>(ItemObject);
	if (ItemData && !ItemData->DisplayName.IsEmpty())
	{
		return ItemData->DisplayName;
	}

	return FText::FromName(ItemId.PrimaryAssetName);
}


EEDItemRarity UEDInventoryQuickBarWidget::ResolveItemRarity(const FPrimaryAssetId& ItemId) const
{
	if (!ItemId.IsValid())
	{
		return EEDItemRarity::Normal;
	}

	UObject* ItemObject = UAssetManager::Get().GetPrimaryAssetObject(ItemId);
	if (!ItemObject)
	{
		const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
		if (AssetPath.IsValid())
		{
			ItemObject = AssetPath.TryLoad();
		}
	}

	const UEDInventoryItemDataAsset* ItemData = Cast<UEDInventoryItemDataAsset>(ItemObject);
	return ItemData ? ItemData->Rarity : EEDItemRarity::Normal;
}

bool UEDInventoryQuickBarWidget::TryGetQuickSlotData(int32 QuickIndex, FEDInventorySlotData& OutSlotData) const
{
	if (!InventoryComponent)
	{
		return false;
	}

	if (!InventoryComponent->InventorySlots.IsValidIndex(QuickIndex))
	{
		return false;
	}

	OutSlotData = InventoryComponent->InventorySlots[QuickIndex];
	return true;
}

void UEDInventoryQuickBarWidget::ShowInventoryFailure(EEDInventoryActionFailure Failure) const
{
	if (!ActionResultText)
	{
		return;
	}

	const FText FailureText = UEDInventoryBlueprintLibrary::GetInventoryActionFailureText(Failure);
	ActionResultText->SetText(FailureText);
	ActionResultText->SetVisibility(ESlateVisibility::Visible);
}

void UEDInventoryQuickBarWidget::ClearInventoryActionMessage() const
{
	if (!ActionResultText)
	{
		return;
	}

	ActionResultText->SetText(FText::GetEmpty());
	ActionResultText->SetVisibility(ESlateVisibility::Collapsed);
}

void UEDInventoryQuickBarWidget::HandleQuickSlotDroppedOnSlot(int32 FromSlotIndex, int32 ToSlotIndex)
{
	TryMoveQuickSlotItem(FromSlotIndex, ToSlotIndex);
}

void UEDInventoryQuickBarWidget::HandleQuickSlotDroppedOutside(int32 FromSlotIndex)
{
	if (!InventoryComponent)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	FEDInventorySlotData SlotData;
	if (!TryGetQuickSlotData(FromSlotIndex, SlotData))
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidSlot);
		return;
	}

	if (SlotData.IsEmpty())
	{
		ShowInventoryFailure(EEDInventoryActionFailure::EmptySlot);
		return;
	}

	// 수량이 1이면 바로 버린다.
	if (SlotData.Item.Quantity <= 1)
	{
		TryDropQuickSlotItemPartial(FromSlotIndex, 1);
		return;
	}

	// 스택 아이템이면 수량 선택 팝업을 띄운다.
	OpenDropQuantityPopup(FromSlotIndex, SlotData.Item.Quantity);
}

void UEDInventoryQuickBarWidget::TryMoveQuickSlotItem(int32 FromSlotIndex, int32 ToSlotIndex)
{
	if (!InventoryComponent)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	if (FromSlotIndex == INDEX_NONE || ToSlotIndex == INDEX_NONE)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidSlot);
		return;
	}

	const bool bSuccess = InventoryComponent->RequestMoveItemBetweenSlots(FromSlotIndex, ToSlotIndex);
	if (!bSuccess)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidSlot);
		return;
	}

	ClearInventoryActionMessage();
}

void UEDInventoryQuickBarWidget::TryDropQuickSlotItemPartial(int32 FromSlotIndex, int32 DropQuantity)
{
	if (!InventoryComponent)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	if (DropQuantity <= 0)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidQuantity);
		return;
	}

	const bool bSuccess = InventoryComponent->RequestDropPartialFromSlot(FromSlotIndex, DropQuantity);
	if (!bSuccess)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidSlot);
		return;
	}

	ClearInventoryActionMessage();
}

void UEDInventoryQuickBarWidget::OpenDropQuantityPopup(int32 FromSlotIndex, int32 MaxQuantity)
{
	if (!QuantityPopupWidget)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::MissingData);
		return;
	}

	PendingDropSlotIndex = FromSlotIndex;
	const int32 SafeMaxQuantity = FMath::Max(1, MaxQuantity);
	QuantityPopupWidget->SetupQuantityRange(1, SafeMaxQuantity, 1);
	ClearInventoryActionMessage();
}

void UEDInventoryQuickBarWidget::CloseDropQuantityPopup()
{
	PendingDropSlotIndex = INDEX_NONE;

	if (QuantityPopupWidget)
	{
		QuantityPopupWidget->ResetPopupState();
	}
}

void UEDInventoryQuickBarWidget::HandleQuantityPopupConfirmed(int32 SelectedQuantity)
{
	if (PendingDropSlotIndex == INDEX_NONE)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidSlot);
		CloseDropQuantityPopup();
		return;
	}

	TryDropQuickSlotItemPartial(PendingDropSlotIndex, SelectedQuantity);
	CloseDropQuantityPopup();
}

void UEDInventoryQuickBarWidget::HandleQuantityPopupCanceled()
{
	CloseDropQuantityPopup();
}

void UEDInventoryQuickBarWidget::HandleQuickSlotClicked(int32 InSlotIndex)
{
	// 좌클릭 - 슬롯 선택 처리
	SelectedSlotIndex = InSlotIndex;
	RefreshSelectedSlotState();
}

void UEDInventoryQuickBarWidget::HandleQuickSlotDoubleClicked(int32 InSlotIndex)
{
	if (!InventoryComponent)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	FEDInventorySlotData SlotData;
	if (!TryGetQuickSlotData(InSlotIndex, SlotData))
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidSlot);
		return;
	}

	if (SlotData.IsEmpty())
	{
		ShowInventoryFailure(EEDInventoryActionFailure::EmptySlot);
		return;
	}
	
	// 기존 Object가져오는 로직에서 ResolveItemData를 통해 캐시된 데이터 반환
	const UEDInventoryItemDataAsset* ItemData = ResolveItemData(SlotData.Item.ItemId);
	if (!ItemData)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::MissingData);
		return;
	}

	// 소비형 아이템 - 즉시 사용
	if (ItemData->ItemType == EEDInventoryItemType::Consumable)
	{
		EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
		const bool bSuccess = InventoryComponent->RequestConsumeItemAtSlotDetailed(InSlotIndex, Failure);

		if (!bSuccess)
		{
			ShowInventoryFailure(Failure);
			return;
		}

		ClearInventoryActionMessage();
		return;
	}

	// 장비형 아이템 - 장착
	if (ItemData->ItemType == EEDInventoryItemType::Equippable)
	{
		if (ItemData->EquippableType == EEDEquippableType::None)
		{
			ShowInventoryFailure(EEDInventoryActionFailure::MissingData);
			return;
		}

		const bool bSuccess = InventoryComponent->RequestEquipItemFromSlot(InSlotIndex, ItemData->EquippableType);
		if (!bSuccess)
		{
			ShowInventoryFailure(EEDInventoryActionFailure::SlotConflict);
			return;
		}

		ClearInventoryActionMessage();
		return;
	}

	// 현재 즉시 사용/장착이 없는 아이템 타입
	ShowInventoryFailure(EEDInventoryActionFailure::NotConsumable);
}

void UEDInventoryQuickBarWidget::RefreshSelectedSlotState()
{
	for (int32 Index = 0; Index < QuickSlotWidgets.Num(); ++Index)
	{
		if (QuickSlotWidgets[Index])
		{
			QuickSlotWidgets[Index]->SetSelectedState(Index == SelectedSlotIndex);
		}
	}
}

const UEDInventoryItemDataAsset* UEDInventoryQuickBarWidget::ResolveItemData(const FPrimaryAssetId& ItemId) const
{
	if (!ItemId.IsValid())
	{
		return nullptr;
	}
	
	const UEDGameDataSubsystem* DS = UEDGameDataSubsystem::Get(this);
	if (!DS) return nullptr;
	
	const UEDInventoryItemDataAsset* Cached = DS->GetData<UEDInventoryItemDataAsset>(ItemId);
	if (Cached)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Cache HIT] %s"), *ItemId.ToString());
		return Cached;
	} else
	{
		// 캐시 미스면 AssetManager의 인메모리 확인
		UE_LOG(LogTemp, Warning, TEXT("[Cache MISS] %s"), *ItemId.ToString());
		return UEDAssetManager::Get().GetPrimaryAsset<UEDInventoryItemDataAsset>(ItemId);
	}
}

void UEDInventoryQuickBarWidget::PreloadInventoryAssets()
{
	if (!InventoryComponent)
	{
		// 인벤토리가 없어도 빈 슬롯 상태로 화면을 표시
		DoRefresh();
		return;
	}
	UEDAssetManager& AM = UEDAssetManager::Get();
	
	// 메모리에 없는 에셋 경로만 수집
	TArray<FSoftObjectPath> PathsToLoad;
	for (const FEDInventorySlotData& InventorySlot : InventoryComponent->InventorySlots)
	{
		if (!InventorySlot.Item.ItemId.IsValid())
		{
			continue;
		}

		// 이미 메모리에 있으므로 스킵
		if (AM.IsPrimaryAssetLoaded(InventorySlot.Item.ItemId))
		{
			continue;
		}

		const FSoftObjectPath Path = AM.GetPrimaryAssetPath(InventorySlot.Item.ItemId);
		if (Path.IsValid())
		{
			PathsToLoad.AddUnique(Path);
		}
	}
	
	// 로드할 에셋이 없으면 즉시 리프레시
	if (PathsToLoad.IsEmpty())
	{
		DoRefresh();
		return;
	}
	PreloadHandle = AM.LoadAssetsAsync(
		PathsToLoad,
		FStreamableDelegate::CreateUObject(this, &UEDInventoryQuickBarWidget::DoRefresh)
	);
}

// 콜백
void UEDInventoryQuickBarWidget::DoRefresh()
{
	bRefreshPending = false;
	RefreshQuickSlots();
	RefreshEquipmentSlots();
}
