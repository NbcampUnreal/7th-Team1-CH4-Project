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
	// 시작 시 콜리전 비활성 — ActivateZone()에서 켜짐
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaMesh->SetGenerateOverlapEvents(true);
	
	// 금지구역 라인
	LineRestrictMesh = CreateDefaultSubobject<UStaticMeshComponent>("LineRestrictMesh");
	LineRestrictMesh->SetupAttachment(RootComponent);
	LineRestrictMesh->SetHiddenInGame(true);
	LineRestrictMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 완전 제거
	LineRestrictMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	LineRestrictMesh->SetGenerateOverlapEvents(false); // 오버랩 이벤트 끄기
	
	// 기본 구역 라인
	LineAreaMesh = CreateDefaultSubobject<UStaticMeshComponent>("LineAreaMesh");
	LineAreaMesh->SetupAttachment(RootComponent);
	LineAreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 완전 제거
	LineAreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	LineAreaMesh->SetGenerateOverlapEvents(false); // 오버랩 이벤트 끄기
	
}

void AEDRestrictedArea::BeginPlay()
{
	Super::BeginPlay();
	AreaMesh->OnComponentBeginOverlap.AddDynamic(this, &AEDRestrictedArea::OnMeshBeginOverlap);
	AreaMesh->OnComponentEndOverlap.AddDynamic(this, &AEDRestrictedArea::OnMeshEndOverlap);

	LineAreaMesh->SetCustomPrimitiveDataVector4(0, DefaultLineColor);
}

void AEDRestrictedArea::ActivateZone()
{
	if (!HasAuthority()) return;
	if (!IsValid(LineRestrictMesh)) return;
	if (!IsValid(LineAreaMesh)) return;
	
	// 콜리전이 이미 켜져있으면 중복 활성화 방지
	if (AreaMesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision) return;

	// 콜리전 활성화
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 금지구역 라인 활성화
	LineRestrictMesh->SetHiddenInGame(false);
	
	// 기본구역 라인 색상변경
	LineAreaMesh->SetCustomPrimitiveDataVector4(0, FLinearColor::Red);
	
	// 오버랩 정보 즉시 갱신 → 이미 안에 있는 플레이어 감지
	AreaMesh->UpdateOverlaps();

	/*
	TArray<AActor*> OverlappingActors;
	AreaMesh->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!IsValid(Actor)) continue;

		UZoneDetectorComponent* ZoneDetector = Actor->FindComponentByClass<UZoneDetectorComponent>();
		if (ZoneDetector)
		{
			ZoneDetector->EnterRestrictedArea(RestrictedAreaEffectClass);
		}
	}
	*/
	
	
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
