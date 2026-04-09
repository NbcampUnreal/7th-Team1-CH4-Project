// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/EDRestrictedArea.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/Player/Component/ZoneDetectorComponent.h"
#include "GameplayEffect.h"

AEDRestrictedArea::AEDRestrictedArea()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>("AreaMesh");
	SetRootComponent(AreaMesh);

	AreaMesh->SetHiddenInGame(true);
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaMesh->SetGenerateOverlapEvents(true);
}

void AEDRestrictedArea::BeginPlay()
{
	Super::BeginPlay();
	AreaMesh->OnComponentBeginOverlap.AddDynamic(this, &AEDRestrictedArea::OnMeshBeginOverlap);
	AreaMesh->OnComponentEndOverlap.AddDynamic(this, &AEDRestrictedArea::OnMeshEndOverlap);
}

void AEDRestrictedArea::OnMeshBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 1. 권한이 없는 클라이언트는 무시 (서버에서만 검사)
	if (!HasAuthority()) return;

	// 2. 유효성 및 루트 컴포넌트 검사
	if (!RestrictedAreaEffectClass || !IsValid(OtherActor)) return;
	if (OtherComp != OtherActor->GetRootComponent()) return;

	// 3. 탐지기 컴포넌트가 달린 액터인지 확인하고 명령 전달
	UZoneDetectorComponent* ZoneDetector = OtherActor->FindComponentByClass<UZoneDetectorComponent>();
	if (ZoneDetector)
	{
		ZoneDetector->EnterRestrictedArea(RestrictedAreaEffectClass);
	}
}

void AEDRestrictedArea::OnMeshEndOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 1. 권한이 없는 클라이언트는 무시
	if (!HasAuthority()) return;

	// 2. 유효성 및 루트 컴포넌트 검사
	if (!RestrictedAreaEffectClass || !IsValid(OtherActor)) return;
	if (OtherComp != OtherActor->GetRootComponent()) return;

	// 3. 탐지기 컴포넌트가 달린 액터인지 확인하고 명령 전달
	UZoneDetectorComponent* ZoneDetector = OtherActor->FindComponentByClass<UZoneDetectorComponent>();
	if (ZoneDetector)
	{
		ZoneDetector->ExitRestrictedArea();
	}
}
