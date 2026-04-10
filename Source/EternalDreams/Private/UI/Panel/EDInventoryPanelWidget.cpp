// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDInventoryPanelWidget.h"

#include "Components/TextBlock.h"

void UEDInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 인벤토리 패널이 생성되었습니다."));
}

void UEDInventoryPanelWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 인벤토리 패널이 열렸습니다."));
}

void UEDInventoryPanelWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UE_LOG(LogTemp, Log, TEXT("EDInventoryPanelWidget: 인벤토리 패널이 닫혔습니다."));
}