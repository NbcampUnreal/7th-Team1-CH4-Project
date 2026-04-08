// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/EDInteractiveTest.h"


// Sets default values
AEDInteractiveTest::AEDInteractiveTest()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetRelativeLocation(FVector(-50.f, -50.f, 50.f));
}

// Called when the game starts or when spawned
void AEDInteractiveTest::BeginPlay()
{
	Super::BeginPlay();

		UE_LOG(LogTemp, Log, TEXT(""));
	
}

