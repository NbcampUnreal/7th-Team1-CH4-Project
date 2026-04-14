// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDInventoryPanelWidget.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Engine/AssetManager.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/Panel/EDInventorySlotWidget.h"

void UEDInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 패널 생성 시 슬롯 위젯만 먼저 준비
	// 실제 표시 대상 인벤토리는 SetDisplayedInventoryComponent로 외부에서 주입
	CreateInventorySlotWidgets();
	RefreshInventorySlots();

	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Loot")));
	}

	if (ContainerNameText)
	{
		ContainerNameText->SetText(FText::FromString(TEXT("Container")));
	}

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 루팅 패널이 생성되었습니다."));
	
	InitializePlayerInventoryComponent();
}

void UEDInventoryPanelWidget::NativeDestruct()
{
	UnbindInventoryChanged();

	Super::NativeDestruct();

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 루팅 패널이 닫혔습니다."));
}

void UEDInventoryPanelWidget::SetDisplayedInventoryComponent(UEDInventoryComponent* InInventoryComponent)
{
	// 표시 대상이 같으면 다시 바인딩하지 않음
	if (DisplayedInventoryComponent == InInventoryComponent)
	{
		RefreshInventorySlots();
		return;
	}

	UnbindInventoryChanged();
	DisplayedInventoryComponent = InInventoryComponent;
	BindInventoryChanged();
	RefreshInventorySlots();

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 표시 대상 인벤토리를 갱신했습니다."));
}

void UEDInventoryPanelWidget::CreateInventorySlotWidgets()
{
	if (!InventoryGrid)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryPanelWidget: InventoryGrid가 바인딩되지 않았습니다."));
		return;
	}

	if (!InventorySlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryPanelWidget: InventorySlotWidgetClass가 설정되지 않았습니다."));
		return;
	}

	InventoryGrid->ClearChildren();
	InventorySlotWidgets.Reset();

	// 루팅 패널은 5 x 2 그리드 기준으로 10칸을 기본 사용
	const int32 SlotCount = 10;

	for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
	{
		UEDInventorySlotWidget* SlotWidget = CreateWidget<UEDInventorySlotWidget>(this, InventorySlotWidgetClass);
		if (!SlotWidget)
		{
			continue;
		}

		InventorySlotWidgets.Add(SlotWidget);

		const int32 Row = SlotsPerRow > 0 ? SlotIndex / SlotsPerRow : 0;
		const int32 Column = SlotsPerRow > 0 ? SlotIndex % SlotsPerRow : SlotIndex;

		if (UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(SlotWidget, Row, Column))
		{
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("LootPanel: Created SlotWidgets=%d"), InventorySlotWidgets.Num());
}

void UEDInventoryPanelWidget::RefreshInventorySlots()
{
	// 표시 대상이 없으면 패널을 빈 슬롯 상태로 유지
	for (int32 Index = 0; Index < InventorySlotWidgets.Num(); ++Index)
	{
		UEDInventorySlotWidget* SlotWidget = InventorySlotWidgets[Index];
		if (!SlotWidget)
		{
			continue;
		}

		FEDInventorySlotData SlotData;
		if (!TryGetSlotData(Index, SlotData) || SlotData.IsEmpty())
		{
			SlotWidget->SetEmptyState();
			continue;
		}

		const FText ItemName = ResolveItemDisplayName(SlotData.Item.ItemId);
		const EEDItemRarity ItemRarity = ResolveItemRarity(SlotData.Item.ItemId);

		SlotWidget->SetItemState(ItemName, SlotData.Item.Quantity, ItemRarity);
	}

	RefreshCapacityText();
}

void UEDInventoryPanelWidget::RefreshCapacityText() const
{
	if (!CapacityText)
	{
		return;
	}

	if (!DisplayedInventoryComponent)
	{
		CapacityText->SetText(FText::FromString(TEXT("0 / 0")));
		return;
	}

	int32 UsedSlotCount = 0;
	for (const FEDInventorySlotData& SlotData : DisplayedInventoryComponent->InventorySlots)
	{
		if (!SlotData.IsEmpty())
		{
			++UsedSlotCount;
		}
	}

	CapacityText->SetText(
		FText::FromString(FString::Printf(TEXT("%d / %d"), UsedSlotCount, DisplayedInventoryComponent->MaxInventorySlots))
	);
}

void UEDInventoryPanelWidget::BindInventoryChanged()
{
	if (!DisplayedInventoryComponent)
	{
		return;
	}

	DisplayedInventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UEDInventoryPanelWidget::HandleInventoryChanged);
}

void UEDInventoryPanelWidget::UnbindInventoryChanged()
{
	if (!DisplayedInventoryComponent)
	{
		return;
	}

	DisplayedInventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UEDInventoryPanelWidget::HandleInventoryChanged);
}

void UEDInventoryPanelWidget::HandleInventoryChanged()
{
	// 표시 중인 외부 인벤토리가 바뀌면 슬롯 전체를 다시 그림
	RefreshInventorySlots();
}

FText UEDInventoryPanelWidget::ResolveItemDisplayName(const FPrimaryAssetId& ItemId) const
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

EEDItemRarity UEDInventoryPanelWidget::ResolveItemRarity(const FPrimaryAssetId& ItemId) const
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

bool UEDInventoryPanelWidget::TryGetSlotData(int32 InSlotIndex, FEDInventorySlotData& OutSlotData) const
{
	if (!DisplayedInventoryComponent)
	{
		return false;
	}

	if (!DisplayedInventoryComponent->InventorySlots.IsValidIndex(InSlotIndex))
	{
		return false;
	}

	OutSlotData = DisplayedInventoryComponent->InventorySlots[InSlotIndex];
	return true;
}

void UEDInventoryPanelWidget::InitializePlayerInventoryComponent()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryPanelWidget: OwningPlayerPawn을 찾을 수 없습니다."));
		return;
	}

	PlayerInventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(OwningPawn);
	if (!PlayerInventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryPanelWidget: PlayerInventoryComponent를 찾을 수 없습니다."));
	}
}

void UEDInventoryPanelWidget::HandleLootSlotDoubleClicked(int32 InSlotIndex)
{
}

void UEDInventoryPanelWidget::TryTransferItemToPlayerInventory(int32 InSlotIndex)
{
}
