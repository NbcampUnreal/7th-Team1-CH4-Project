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
		PlayerController = Cast<AEDPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		if (!IsValid(PlayerController))
		{
			return;
		}
	}

	// 부활 시 새 Pawn으로 교체되면 캐시 갱신 + LookTarget 재센터
	APawn* CurrentPawn = PlayerController->GetPawn();
	if (IsValid(CurrentPawn) && CurrentPawn != PlayerCharacter)
	{
		PlayerCharacter = Cast<AEDPlayerCharacter>(CurrentPawn);
		if (IsValid(PlayerCharacter))
		{
			LookTargetLocation = PlayerCharacter->GetActorLocation();
		}
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
		LookTargetLocation=PlayerCharacter->GetActorLocation();

	}
	else
	{
		CameraMove(DeltaTime);
	}
	SetActorLocation(LookTargetLocation+CameraOffset);
}

void AEDCameraActor::CameraZoom(FInputActionValue value)
{
	FVector BufferVector;
	BufferVector=CameraOffset+(CameraFrontVector*CameraScrollSpeed*(value.Get<float>()));
	

	
	CameraOffset=FVector(
		//카메라 오프셋의 X좌표는 음수이므로, MinZoom(최대 줌아웃)이 절대값이 커서 Min에 위치한다.
		FMath::Clamp(BufferVector.X,MinZoomCameraOffset.X,MaxZoomCameraOffset.X),
		CameraOffset.Y,
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
	else
	{
		CameraOffset=MinZoomCameraOffset;
		bIsFocusedPlayer=true;
	}

}

void AEDCameraActor::CameraMove(float DeltaTime)
{
	if (!GEngine||!GEngine->GameViewport)
	{
		return;
	}
	float BufferX, BufferY;
	FVector2D ViewportSize;
	//현재 마우스의 픽셀 단위 위치를 구한다.
	PlayerController->GetMousePosition(BufferX,BufferY);
	FVector2D MousePos=FVector2D(BufferX,BufferY);
	//뷰포트의 크기를 픽셀 단위로 구한다.
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	//정규화된 마우스의 위치값
	FVector2D NormalPosition=MousePos/ViewportSize;
	//마우스의 위치에 따른 방향벡터
	FVector DirVector=FVector::ZeroVector;
	
	NormalCameraMoveEdge=FMath::Clamp(NormalCameraMoveEdge,0.0f,0.5f);
	if (NormalPosition.X<NormalCameraMoveEdge)
	{
		DirVector+=FVector(0.0f,-1.f,0.f);
	}
	if (NormalPosition.X>(1.0f-NormalCameraMoveEdge))
	{
		DirVector+=FVector(0.0f,1.f,0.f);
	}
	if (NormalPosition.Y<NormalCameraMoveEdge)
	{
		DirVector+=FVector(1.0f,0.f,0.f);
	}
	if (NormalPosition.Y>(1.0f-NormalCameraMoveEdge))
	{
		DirVector+=FVector(-1.0f,0.f,0.f);
	}
	
	FVector BufferVector=LookTargetLocation+(DirVector*CameraMoveSpeed*DeltaTime);
	//TODO: 카메라가 맵을 벗어나지 못하게 처리
	
	LookTargetLocation=BufferVector;
}

