// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/OtherActor/EDCursorActor.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"


// Sets default values
AEDCursorActor::AEDCursorActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Scene=CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(Scene);
	/*CursorWidget=CreateDefaultSubobject<UWidgetComponent>(TEXT("CursorWidget"));
	CursorWidget->SetupAttachment(Scene);
	CursorWidget->SetWidgetSpace(EWidgetSpace::Screen);
	*/
	
}

// Called when the game starts or when spawned
void AEDCursorActor::BeginPlay()
{
	Super::BeginPlay();
	if (CursorWidget!=nullptr)
	{
		// 커서 위젯은 항상 최상단에 그리되, 마우스 히트 테스트는 막지 않아야
		// 인벤토리 슬롯의 클릭 / 더블클릭 / 드래그 입력을 가로채지 않음
		CursorWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		CursorWidget->AddToViewport(999);
	}

}

// Called every frame
void AEDCursorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CursorWidget==nullptr)
	{
		return;
	}
	
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
	/*
	//위치 변경
	FHitResult HitResult;
	if (PlayerController->GetHitResultUnderCursor(ECC_Visibility,false, HitResult))
	{
		if (IsValid(CursorWidget))
		{
			CursorWidget->SetWorldLocation(HitResult.ImpactPoint);
		}
	}
	*/
	float MouseX, MouseY;
	PlayerController->GetMousePosition(MouseX,MouseY);
	FVector2D MousePos=FVector2D(MouseX,MouseY);

	CursorWidget->SetPositionInViewport(MousePos,true);
	
}
