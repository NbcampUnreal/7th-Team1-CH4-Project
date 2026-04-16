// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/EDTestLootContainer.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Interaction/Component/EDLootTargetComponent.h"

AEDTestLootContainer::AEDTestLootContainer()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	SetReplicateMovement(false);

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetRootComponent(InteractionBox);

	// 루팅 가능한 범위 설정
	InteractionBox->SetBoxExtent(FVector(120.0f, 120.0f, 120.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(InteractionBox);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InventoryComponent = CreateDefaultSubobject<UEDInventoryComponent>(TEXT("InventoryComponent"));
	if (InventoryComponent)
	{
		// 외부 컨테이너이므로 장비 슬롯/기본 무기 지급은 사용하지 않음
		InventoryComponent->bUseEquipmentSlots = false;
		InventoryComponent->bGiveDefaultWeaponOnBeginPlay = false;
		InventoryComponent->MaxInventorySlots = 10;
	}

	LootTargetComponent = CreateDefaultSubobject<UEDLootTargetComponent>(TEXT("LootTargetComponent"));
	if (LootTargetComponent)
	{
		LootTargetComponent->SetInteractionCollision(InteractionBox);
	}
}

UEDInventoryComponent* AEDTestLootContainer::GetInventoryComponent() const
{
	return InventoryComponent;
}
