// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/OtherActor/EDCameraActor.h"

#include "Camera/CameraComponent.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/EDPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Slate/SGameLayerManager.h"


// Sets default values
AEDCameraActor::AEDCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SpringArm=CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = SpringArmLength;
	SetRootComponent(SpringArm);
	Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void AEDCameraActor::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter=Cast<AEDPlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	SetActorLocation(PlayerCharacter->GetActorLocation()+CameraOffset);
	
	SetActorRotation(CameraRotation);
}

// Called every frame
void AEDCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(bIsFocusedPlayer)
	{
		if (!IsValid(PlayerCharacter))
		{
			PlayerCharacter=Cast<AEDPlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
			if (!IsValid(PlayerCharacter))
			{
				return;
			}
		}
		AEDPlayerController* PC=Cast<AEDPlayerController>(PlayerCharacter->GetController());
		
		PC->SetViewTargetWithBlend(this);
		SetActorLocation(PlayerCharacter->GetActorLocation()+CameraOffset);
	}
}

