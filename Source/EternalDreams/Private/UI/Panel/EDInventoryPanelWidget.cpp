// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDInventoryPanelWidget.h"

#include "Characters/Player/Component/EDLootInteractionComponent.h"
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

void UEDInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 루팅 패널 생성 시 슬롯 위젯을 먼저 준비
	// 실제 표시 대상 인벤토리는 외부에서 SetDisplayedInventoryComponent로 주입
	CreateInventorySlotWidgets();
	InitializePlayerInventoryComponent();
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
}

void UEDInventoryPanelWidget::NativeDestruct()
{
	UnbindInventoryChanged();

	Super::NativeDestruct();

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 루팅 패널이 닫혔습니다."));
}

void UEDInventoryPanelWidget::SetDisplayedInventoryComponent(UEDInventoryComponent* InInventoryComponent)
{
	// 표시 대상이 같으면 다시 바인딩하지 않고 화면만 갱신
	if (DisplayedInventoryComponent == InInventoryComponent)
	{
		RefreshInventorySlots();
		return;
	}

	UnbindInventoryChanged();
	DisplayedInventoryComponent = InInventoryComponent;
	BindInventoryChanged();
	RefreshInventorySlots();

	// 외부 컨테이너 이름이 있으면 패널에 반영
	if (ContainerNameText)
	{
		if (DisplayedInventoryComponent && DisplayedInventoryComponent->GetOwner())
		{
			ContainerNameText->SetText(FText::FromString(DisplayedInventoryComponent->GetOwner()->GetName()));
		}
		else
		{
			ContainerNameText->SetText(FText::FromString(TEXT("Container")));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 표시 대상 인벤토리를 갱신했습니다. Owner=%s"),
	       DisplayedInventoryComponent && DisplayedInventoryComponent->GetOwner()
	       ? *DisplayedInventoryComponent->GetOwner()->GetName()
	       : TEXT("None"));
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

		// 슬롯 인덱스를 부여하고 더블 클릭 이동 이벤트 연결
		SlotWidget->SetSlotIndex(SlotIndex);
		SlotWidget->OnSlotClicked.AddUObject(this, &UEDInventoryPanelWidget::HandleLootSlotClicked);
		SlotWidget->OnSlotDoubleClicked.AddUObject(this, &UEDInventoryPanelWidget::HandleLootSlotDoubleClicked);
	}
}

void UEDInventoryPanelWidget::RefreshInventorySlots()
{
	UE_LOG(LogTemp, Warning, TEXT("LootPanel: Refresh Start Owner=%s Slots=%d WidgetCount=%d"),
	       DisplayedInventoryComponent && DisplayedInventoryComponent->GetOwner()
	       ? *DisplayedInventoryComponent->GetOwner()->GetName()
	       : TEXT("None"),
	       DisplayedInventoryComponent ? DisplayedInventoryComponent->InventorySlots.Num() : -1,
	       InventorySlotWidgets.Num());

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
			UE_LOG(LogTemp, Warning, TEXT("LootPanel: Slot %d read failed"), Index);
			continue;
		}

		if (SlotData.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("LootPanel: Slot %d empty"), Index);
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("LootPanel: Slot %d ItemId=%s Quantity=%d"),
		       Index,
		       *SlotData.Item.ItemId.ToString(),
		       SlotData.Item.Quantity);

		const FText ItemName = ResolveItemDisplayName(SlotData.Item.ItemId);
		const EEDItemRarity ItemRarity = ResolveItemRarity(SlotData.Item.ItemId);

		SlotWidget->SetItemState(ItemName, SlotData.Item.Quantity, ItemRarity);
	}

	RefreshCapacityText();
	RefreshSelectedSlotState();
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
		FText::FromString(FString::Printf(TEXT("%d / %d"), UsedSlotCount,
		                                  DisplayedInventoryComponent->MaxInventorySlots))
	);
}

void UEDInventoryPanelWidget::BindInventoryChanged()
{
	if (!DisplayedInventoryComponent)
	{
		return;
	}

	DisplayedInventoryComponent->OnInventoryChanged.AddUniqueDynamic(
		this, &UEDInventoryPanelWidget::HandleInventoryChanged);
}

void UEDInventoryPanelWidget::UnbindInventoryChanged()
{
	if (!DisplayedInventoryComponent)
	{
		return;
	}

	DisplayedInventoryComponent->OnInventoryChanged.RemoveDynamic(
		this, &UEDInventoryPanelWidget::HandleInventoryChanged);
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

void UEDInventoryPanelWidget::HandleInventoryChanged()
{
	// 외부 컨테이너 인벤토리 변경 시 슬롯 전체를 다시 그림
	RefreshInventorySlots();
}

void UEDInventoryPanelWidget::HandleLootSlotDoubleClicked(int32 InSlotIndex)
{
	// 더블 클릭 - 아이템을 플레이어 인벤토리로 옮김
	UE_LOG(LogTemp, Warning, TEXT("LootPanel: DoubleClicked Slot=%d"), InSlotIndex);
	TryTransferItemToPlayerInventory(InSlotIndex);
}

void UEDInventoryPanelWidget::TryTransferItemToPlayerInventory(int32 InSlotIndex)
{
	if (!DisplayedInventoryComponent)
	{
		return;
	}

	FEDInventorySlotData SlotData;
	if (!TryGetSlotData(InSlotIndex, SlotData))
	{
		return;
	}

	if (SlotData.IsEmpty())
	{
		return;
	}

	UEDLootInteractionComponent* LootInteractionComponent = GetLootInteractionComponent();
	if (!LootInteractionComponent)
	{
		return;
	}

	LootInteractionComponent->RequestLootTransfer(
		DisplayedInventoryComponent,
		InSlotIndex,
		SlotData.Item.Quantity
	);
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

UEDLootInteractionComponent* UEDInventoryPanelWidget::GetLootInteractionComponent() const
{
	APlayerController* OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer)
	{
		return nullptr;
	}

	return OwningPlayer->FindComponentByClass<UEDLootInteractionComponent>();
}

void UEDInventoryPanelWidget::HandleLootSlotClicked(int32 InSlotIndex)
{
	SelectedSlotIndex = InSlotIndex;
	RefreshSelectedSlotState();
}

void UEDInventoryPanelWidget::RefreshSelectedSlotState()
{
	for (int32 Index = 0; Index < InventorySlotWidgets.Num(); ++Index)
	{
		if (InventorySlotWidgets[Index])
		{
			InventorySlotWidgets[Index]->SetSelectedState(Index == SelectedSlotIndex);
		}
	}
}
