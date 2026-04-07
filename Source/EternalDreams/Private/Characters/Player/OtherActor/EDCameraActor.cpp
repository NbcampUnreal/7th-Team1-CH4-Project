// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/OtherActor/EDCameraActor.h"

#include "InputActionValue.h"
#include "Blueprint/WidgetLayoutLibrary.h"
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
	/*
	SpringArm=CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = SpringArmLength;
	SetRootComponent(SpringArm);
	*/
	Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	//Camera->SetupAttachment(SpringArm);
	SetRootComponent(Camera);

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
	
	
	PlayerCharacter=Cast<AEDPlayerCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
	PlayerController=Cast<AEDPlayerController>(PlayerCharacter->GetController());
	CameraFrontVector=GetActorForwardVector();
	MinZoomCameraOffset=CameraOffset;
}

// Called every frame
void AEDCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsValid(PlayerController))
	{
		return;
	}
	
	if (PlayerController->GetViewTarget()!=this)
	{
		PlayerController->SetViewTargetWithBlend(this);
	}

	
	if(bIsFocusedPlayer)
	{
		if (!IsValid(PlayerCharacter))
		{
			return;
		}
		
		PlayerController->SetViewTargetWithBlend(this);
		SetActorLocation(PlayerCharacter->GetActorLocation()+CameraOffset);
	}
}

void AEDCameraActor::CameraZoom(FInputActionValue value)
{
	BufferVector=CameraOffset+(CameraFrontVector*CameraScrollSpeed*(value.Get<float>()));
	

	
	CameraOffset=FVector(
		//카메라 오프셋의 X좌표는 음수이므로, MinZoom(최대 줌아웃)이 절대값이 커서 Min에 위치한다.
		FMath::Clamp(BufferVector.X,MinZoomCameraOffset.X,MaxZoomCameraOffset.X),
		0.f,
		//카메라 오프셋의 Z좌표는 양수이므로, MinZoom(최대 줌아웃)이 값이 커서 Max에 위치한다.
		FMath::Clamp(BufferVector.Z,MaxZoomCameraOffset.Z,MinZoomCameraOffset.Z)
		);
	

}

void AEDCameraActor::ToggleCameraFocus(FInputActionValue value)
{
	if (bIsFocusedPlayer)
	{
		bIsFocusedPlayer=false;
		return;
	}
	bIsFocusedPlayer=true;
}

void AEDCameraActor::CameraMove(FInputActionValue value)
{
	FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetWorld());
	FVector2D Viewport= UWidgetLayoutLibrary::GetViewportSize(GetWorld());
	
	
	UE_LOG(LogTemp,Warning,TEXT("%s"),*(MousePos/Viewport).ToString());
}

