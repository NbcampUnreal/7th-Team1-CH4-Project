// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_UpdateTarget.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Data/EDMonsterDataAsset.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	NodeName = TEXT("Update Target");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (IsValid(BB) == false)
		return;
	// Target이 없으면 TargetActor & bIsTracking 초기화
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (IsValid(Target) == false)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
		BB->SetValueAsBool(TEXT("bIsTracking"), false);
		return;
	}
	// ASC로 타겟 HP 조회
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Target);
	if (ASCInterface == nullptr)
		return;
	
	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return;
	// TargetActor가 사망 했다면  초기화
	float Health = ASC->GetNumericAttribute(UEDBaseAttributeSet::GetHealthAttribute());
	if (Health <= 0.f)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
		BB->SetValueAsBool(TEXT("bIsTracking"), false);
		return;
	}
	// 감지 범위 이탈 체크
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
	{
		// TargetActor가 유효하면 추적 중 유지
		BB->SetValueAsBool(TEXT("bIsTracking"), true);
		return;
	}
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false || IsValid(Monster->GetDataAsset()) == false)
	{
		// TargetActor가 유효하면 추적 중 유지
		BB->SetValueAsBool(TEXT("bIsTracking"), true);
		return;
	}
	
	float DetectRange = Monster->GetDataAsset()->GetStat().DetectRange;
	float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	if (Distance > DetectRange * 1.5f)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
		BB->SetValueAsBool(TEXT("bIsTracking"), false);
		return;
	}
	
	BB->SetValueAsBool(TEXT("bIsTracking"), true);
}
