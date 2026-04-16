// Copyright Epic Games, Inc. All Rights Reserved.
#include "Interaction/Component/EDLootTargetComponent.h"

#include "Characters/Player/Component/EDLootInteractionComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UEDLootTargetComponent::UEDLootTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEDLootTargetComponent::SetInteractionCollision(UPrimitiveComponent* InInteractionCollision)
{
	if (InteractionCollision == InInteractionCollision)
	{
		return;
	}

	UnbindInteractionCollision();
	InteractionCollision = InInteractionCollision;
	BindInteractionCollision();
}

void UEDLootTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	BindInteractionCollision();
}

void UEDLootTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindInteractionCollision();

	Super::EndPlay(EndPlayReason);
}

void UEDLootTargetComponent::HandleInteractionBeginOverlap(
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

	LootInteractionComponent->SetCurrentLootTarget(GetOwner());
}

void UEDLootTargetComponent::HandleInteractionEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
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

	LootInteractionComponent->ClearCurrentLootTarget(GetOwner());
}

void UEDLootTargetComponent::BindInteractionCollision()
{
	UPrimitiveComponent* CollisionComponent = ResolveInteractionCollision();
	if (!CollisionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDLootTargetComponent: 바인딩할 충돌 컴포넌트를 찾지 못했습니다. Owner=%s"),
			GetOwner() ? *GetOwner()->GetName() : TEXT("None"));
		return;
	}

	CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionBeginOverlap);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &UEDLootTargetComponent::HandleInteractionBeginOverlap);

	CollisionComponent->OnComponentEndOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionEndOverlap);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &UEDLootTargetComponent::HandleInteractionEndOverlap);
}

void UEDLootTargetComponent::UnbindInteractionCollision()
{
	UPrimitiveComponent* CollisionComponent = ResolveInteractionCollision();
	if (!CollisionComponent)
	{
		return;
	}

	CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionBeginOverlap);
	CollisionComponent->OnComponentEndOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionEndOverlap);
}

UPrimitiveComponent* UEDLootTargetComponent::ResolveInteractionCollision() const
{
	if (InteractionCollision)
	{
		return InteractionCollision;
	}

	if (!bUseOwnerRootPrimitiveWhenEmpty || !GetOwner())
	{
		return nullptr;
	}

	return Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
}
