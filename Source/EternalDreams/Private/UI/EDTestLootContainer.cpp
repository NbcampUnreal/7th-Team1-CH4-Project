// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/EDTestLootContainer.h"

#include "Characters/Player/Component/EDLootInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Inventory/Component/EDInventoryComponent.h"

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
		
		// 루팅 테스트용 더미 아이템 지급 강제
		InventoryComponent->bGiveDebugItemsOnBeginPlay = true;
		InventoryComponent->DebugConsumableItemId = FPrimaryAssetId(TEXT("InventoryItem"), TEXT("DA_ConsumableTest1"));
		InventoryComponent->DebugMaterialItemId = FPrimaryAssetId(TEXT("InventoryItem"), TEXT("DA_IngredientTest1"));
		InventoryComponent->DebugEquipItemId = FPrimaryAssetId(TEXT("InventoryItem"), TEXT("DA_BotArmorTest1"));
	}
}

UEDInventoryComponent* AEDTestLootContainer::GetInventoryComponent() const
{
	return InventoryComponent;
}

void AEDTestLootContainer::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionBox)
	{
		InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AEDTestLootContainer::HandleInteractionBeginOverlap);
		InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AEDTestLootContainer::HandleInteractionEndOverlap);
	}
}

void AEDTestLootContainer::HandleInteractionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	APawn* OverlapPawn = Cast<APawn>(OtherActor);
	if (!OverlapPawn)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OverlapPawn->GetController());
	if (!PlayerController)
	{
		return;
	}

	UEDLootInteractionComponent* LootInteractionComponent =
		PlayerController->FindComponentByClass<UEDLootInteractionComponent>();
	if (!LootInteractionComponent)
	{
		return;
	}

	// 플레이어가 범위 안에 들어오면 현재 루팅 대상으로 등록
	LootInteractionComponent->SetCurrentLootTarget(this);
}

void AEDTestLootContainer::HandleInteractionEndOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* 
	OtherComp, 
	int32 OtherBodyIndex)
{
	APawn* OverlapPawn = Cast<APawn>(OtherActor);
	if (!OverlapPawn)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OverlapPawn->GetController());
	if (!PlayerController)
	{
		return;
	}

	UEDLootInteractionComponent* LootInteractionComponent =
		PlayerController->FindComponentByClass<UEDLootInteractionComponent>();
	if (!LootInteractionComponent)
	{
		return;
	}

	// 이 컨테이너가 현재 루팅 대상일 때만 해제
	LootInteractionComponent->ClearCurrentLootTarget(this);
}
