// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/EDRestrictedArea.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "DrawDebugHelpers.h"

AEDRestrictedArea::AEDRestrictedArea()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>("AreaMesh");
	SetRootComponent(AreaMesh);

	// 1. 인게임에서 메시를 숨깁니다 (투명화)
	AreaMesh->SetHiddenInGame(true);

	// 2. 콜리전 세팅: 오버랩만 허용하도록 설정
	AreaMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 물리 연산 끄기
	AreaMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaMesh->SetGenerateOverlapEvents(true); // 오버랩 이벤트 켜기
}

void AEDRestrictedArea::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		// Box가 아닌 Mesh의 오버랩 이벤트에 바인딩
		AreaMesh->OnComponentBeginOverlap.AddDynamic(this, &AEDRestrictedArea::OnMeshBeginOverlap);
		AreaMesh->OnComponentEndOverlap.AddDynamic(this, &AEDRestrictedArea::OnMeshEndOverlap);
	}
}

void AEDRestrictedArea::OnMeshBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 1. 기본 유효성 검사 ( OtherActor 널 체크)
	if (!HasAuthority() || !RestrictedAreaEffectClass || !IsValid(OtherActor)) return;

	// 2. 중복 방지: 닿은 컴포넌트가 루트 컴포넌트가 아니면 무시
	if (OtherComp != OtherActor->GetRootComponent()) return;

	// 3. ASC 가져오기 및 유효성 검사 (얼리 리턴)
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);
	if (!TargetASC) return;

	// 4. GE 스펙 생성 및 유효성 검사 (얼리 리턴)
	FGameplayEffectContextHandle ContextHandle = TargetASC->MakeEffectContext();
	ContextHandle.AddInstigator(this, this);
	FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(RestrictedAreaEffectClass, 1.0f, ContextHandle);

	if (!SpecHandle.IsValid()) return;

	// 5. 실제 이펙트 적용
	TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	// 6. 결과 검증 및 디버그 출력
	if (GEngine)
	{
		FGameplayTag TargetTag = FGameplayTag::RequestGameplayTag(FName("State.Debuff.RestrictedArea"));
		if (TargetASC->HasMatchingGameplayTag(TargetTag))
		{
			FString Msg = FString::Printf(TEXT("금지구역 진입!\n%s"), *TargetTag.ToString());
			// Z축을 120으로 살짝 더 높이고, 맨 끝의 폰트 스케일을 1.5f -> 1.2f 로 줄였습니다.
			DrawDebugString(GetWorld(), OtherActor->GetActorLocation() + FVector(0, 0, 300), Msg, nullptr, FColor::Red,
			                3.0f, true, 1.2f);
		}
		else
		{
			FString Msg = FString::Printf(TEXT("❌ [오류] 진입 이펙트 적용 실패! (태그 없음: %s)"), *TargetTag.ToString());
			// 캐릭터 위치에서 위로(Z축) 100만큼 띄워서 3초 동안 출력
			DrawDebugString(GetWorld(), OtherActor->GetActorLocation() + FVector(0, 0, 120), Msg, nullptr, FColor::Red,
			                3.0f, true, 1.2f);
		}
	}
}

void AEDRestrictedArea::OnMeshEndOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 1. 기본 유효성 검사 (OtherActor 널 체크)
	if (!HasAuthority() || !RestrictedAreaEffectClass || !IsValid(OtherActor)) return;

	// 2. 루트 컴포넌트인지 확인
	if (OtherComp != OtherActor->GetRootComponent()) return;

	// 3. ASC 가져오기 및 유효성 검사 (얼리 리턴)
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);
	if (!TargetASC) return;

	// 4. 이펙트 지우기
	TargetASC->RemoveActiveGameplayEffectBySourceEffect(RestrictedAreaEffectClass, nullptr, -1);

	// 5. 결과 검증 및 디버그 출력
	if (GEngine)
	{
		FGameplayTag TargetTag = FGameplayTag::RequestGameplayTag(FName("State.Debuff.RestrictedArea"));
		if (!TargetASC->HasMatchingGameplayTag(TargetTag))
		{
			FString Msg = FString::Printf(TEXT("금지구역 이탈!\n%s"), *TargetTag.ToString());
			DrawDebugString(GetWorld(), OtherActor->GetActorLocation() + FVector(0, 0, 100), Msg, nullptr,
			                FColor::Green, 3.0f, true, 1.2f);
		}
		else
		{
			FString Msg = FString::Printf(TEXT("❌ [오류] 이탈 시 태그 제거 실패! (태그 잔존: %s)"), *TargetTag.ToString());
			DrawDebugString(GetWorld(), OtherActor->GetActorLocation() + FVector(0, 0, 120), Msg, nullptr,
			                FColor::Green, 3.0f, true, 1.2f);
		}
	}
}
