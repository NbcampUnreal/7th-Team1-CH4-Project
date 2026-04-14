// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/EDTestLootContainer.h"

#include "Components/StaticMeshComponent.h"
#include "Inventory/Component/EDInventoryComponent.h"

AEDTestLootContainer::AEDTestLootContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	InventoryComponent = CreateDefaultSubobject<UEDInventoryComponent>(TEXT("InventoryComponent"));
	
	if (InventoryComponent)
	{
		InventoryComponent->bUseEquipmentSlots = false;
		InventoryComponent->MaxInventorySlots = 10;

		// BeginPlay 시 더미 아이템 지급 허용
		InventoryComponent->bGiveDebugItemsOnBeginPlay = true;
	}
}
