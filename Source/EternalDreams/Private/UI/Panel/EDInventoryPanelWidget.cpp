// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDInventoryPanelWidget.h"

#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/Panel/EDInventorySlotWidget.h"
#include "UI/Panel/EDEquipmentSlotWidget.h"

void UEDInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 패널 생성 시 플레이어 인벤토리와 연결하고 슬롯 UI를 준비
	InitializeInventoryComponent();
	CreateInventorySlotWidgets();
	RefreshInventorySlots();
	RefreshEquipmentSlots();
	BindInventoryChanged();

	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Inventory")));
	}

	if (HintText)
	{
		HintText->SetText(FText::FromString(TEXT("I : 열기/닫기\n우클릭 : 사용 / 장착 / 버리기")));
	}

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 인벤토리 패널이 생성되었습니다."));
}

void UEDInventoryPanelWidget::NativeDestruct()
{
	UnbindInventoryChanged();

	Super::NativeOnActivated();

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 인벤토리 패널이 열렸습니다."));
}

void UEDInventoryPanelWidget::InitializeInventoryComponent()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryPanelWidget: OwningPlayerPawn을 찾을 수 없습니다."));
		return;
	}

	InventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(OwningPawn);
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDInventoryPanelWidget: InventoryComponent를 찾을 수 없습니다."));
	}
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

	const int32 SlotCount = InventoryComponent ? InventoryComponent->MaxInventorySlots : 20;

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
}

void UEDInventoryPanelWidget::RefreshInventorySlots()
{
	if (!InventoryComponent)
	{
		return;
	}

	for (int32 Index = 0; Index < InventorySlotWidgets.Num(); ++Index)
	{
		UEDInventorySlotWidget* SlotWidget = InventorySlotWidgets[Index];
		if (!SlotWidget)
		{
			continue;
		}

		if (!InventoryComponent->InventorySlots.IsValidIndex(Index))
		{
			SlotWidget->SetEmptyState();
			continue;
		}

		const FEDInventorySlotData& SlotData = InventoryComponent->InventorySlots[Index];
		if (SlotData.IsEmpty())
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
	if (!CapacityText || !InventoryComponent)
	{
		return;
	}

	int32 UsedSlotCount = 0;
	for (const FEDInventorySlotData& SlotData : InventoryComponent->InventorySlots)
	{
		if (!SlotData.IsEmpty())
		{
			++UsedSlotCount;
		}
	}

	CapacityText->SetText(
		FText::FromString(FString::Printf(TEXT("%d / %d"), UsedSlotCount, InventoryComponent->MaxInventorySlots))
	);
}

void UEDInventoryPanelWidget::BindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UEDInventoryPanelWidget::HandleInventoryChanged);
}

void UEDInventoryPanelWidget::UnbindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UEDInventoryPanelWidget::HandleInventoryChanged);
}

void UEDInventoryPanelWidget::HandleInventoryChanged()
{
	// 인벤토리 내용이 바뀌면 슬롯 전체를 다시 그림
	RefreshInventorySlots();
	RefreshEquipmentSlots();
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

void UEDInventoryPanelWidget::RefreshEquipmentSlots()
{
	if (!InventoryComponent)
	{
		return;
	}

	if (WeaponSlotWidget)
	{
		if (InventoryComponent->WeaponSlot.EquippedItem.IsValid())
		{
			const FText ItemName = ResolveItemDisplayName(InventoryComponent->WeaponSlot.EquippedItem.ItemId);
			const EEDItemRarity ItemRarity = ResolveItemRarity(InventoryComponent->WeaponSlot.EquippedItem.ItemId);
			WeaponSlotWidget->SetItemState(FText::FromString(TEXT("Weapon")), ItemName, ItemRarity);
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
			const FText ItemName = ResolveItemDisplayName(InventoryComponent->TopArmorSlot.EquippedItem.ItemId);
			const EEDItemRarity ItemRarity = ResolveItemRarity(InventoryComponent->TopArmorSlot.EquippedItem.ItemId);
			TopArmorSlotWidget->SetItemState(FText::FromString(TEXT("Top Armor")), ItemName, ItemRarity);
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
			const FText ItemName = ResolveItemDisplayName(InventoryComponent->BottomArmorSlot.EquippedItem.ItemId);
			const EEDItemRarity ItemRarity = ResolveItemRarity(InventoryComponent->BottomArmorSlot.EquippedItem.ItemId);
			BottomArmorSlotWidget->SetItemState(FText::FromString(TEXT("Bottom Armor")), ItemName, ItemRarity);
		}
		else
		{
			BottomArmorSlotWidget->SetEmptyState(FText::FromString(TEXT("Bottom Armor")));
		}
	}
}
