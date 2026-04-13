// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_IsInRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Data/EDMonsterDataAsset.h"

UBTDecorator_IsInRange::UBTDecorator_IsInRange()
{
	NodeName = TEXT("Is In Attack Range");
}

bool UBTDecorator_IsInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
		return false;
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false || IsValid(Monster->GetDataAsset()) == false)
		return false;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (IsValid(BB) == false)
		return false;
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (IsValid(Target) == false)
		return false;
	// Monster DataAsset의 AtkRange
	const float AtkRange = Monster->GetDataAsset()->GetStat().AtkRange;
	// Monster의 현재 위치와 Target의 현재 위치의 거리 차이
	const float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	// Monster의 AtkRange 내에 TargetActor가 있는지 아닌지 반환
	return Distance <= AtkRange;
}
