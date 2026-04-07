// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/OtherActor/EDCursorActor.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AEDCursorActor::AEDCursorActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Scene=CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(Scene);
	CursorWidget=CreateDefaultSubobject<UWidgetComponent>(TEXT("CursorWidget"));
	CursorWidget->SetupAttachment(Scene);
	CursorWidget->SetWidgetSpace(EWidgetSpace::Screen);
}

// Called when the game starts or when spawned
void AEDCursorActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEDCursorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!IsValid(PlayerController))
	{
		if (IsValid(GetWorld()))
		{
			PlayerController=GetWorld()->GetFirstPlayerController();
		}
		if (!IsValid(PlayerController))
		{
			return;
		}
	}
	
	//위치 변경
	FHitResult HitResult;
	if (PlayerController->GetHitResultUnderCursor(ECC_Visibility,false, HitResult))
	{
		if (IsValid(CursorWidget))
		{
			CursorWidget->SetWorldLocation(HitResult.ImpactPoint);
		}
	}
}

