// Copyright Epic Games, Inc. All Rights Reserved.
#include "Interaction/Component/EDLootTargetComponent.h"

#include "Characters/Player/Component/EDLootInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UEDLootTargetComponent::UEDLootTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEDLootTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateInteractionBox();
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

void UEDLootTargetComponent::CreateInteractionBox()
{
	if (InteractionBox || !GetOwner())
	{
		return;
	}
	
	InteractionBox = NewObject<UBoxComponent>(GetOwner(), TEXT("LootTargetInteractionBox"));
	if (!InteractionBox)
	{
		return;
	}

	GetOwner()->AddInstanceComponent(InteractionBox);

	if (USceneComponent* RootComponent = GetOwner()->GetRootComponent())
	{
		InteractionBox->SetupAttachment(RootComponent);
	}

	InteractionBox->SetRelativeLocation(TriggerRelativeLocation);
	InteractionBox->SetBoxExtent(TriggerBoxExtent);
	InteractionBox->SetGenerateOverlapEvents(true);
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionBox->RegisterComponent();

	if (!GetOwner()->GetRootComponent())
	{
		GetOwner()->SetRootComponent(InteractionBox);
	}
}

void UEDLootTargetComponent::BindInteractionCollision()
{
	if (!InteractionBox)
	{
		return;
	}

	InteractionBox->OnComponentBeginOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionBeginOverlap);
	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &UEDLootTargetComponent::HandleInteractionBeginOverlap);

	InteractionBox->OnComponentEndOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionEndOverlap);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &UEDLootTargetComponent::HandleInteractionEndOverlap);
}

void UEDLootTargetComponent::UnbindInteractionCollision()
{
	if (!InteractionBox)
	{
		return;
	}

	InteractionBox->OnComponentBeginOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionBeginOverlap);
	InteractionBox->OnComponentEndOverlap.RemoveDynamic(this, &UEDLootTargetComponent::HandleInteractionEndOverlap);
}
