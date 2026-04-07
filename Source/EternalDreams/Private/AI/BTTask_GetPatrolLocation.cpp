// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_GetPatrolLocation.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Monster/EDMonsterBase.h"

UBTTask_GetPatrolLocation::UBTTask_GetPatrolLocation()
{
	NodeName = TEXT("Get Patrol Location");
}

EBTNodeResult::Type UBTTask_GetPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
		return EBTNodeResult::Failed;
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false)
		return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (IsValid(BB) == false)
		return EBTNodeResult::Failed;
	
	FVector2D RandPoint = FMath::RandPointInCircle(PatrolRadius);
	FVector PatrolLocation = Monster->GetOriginLocation() + FVector(RandPoint.X, RandPoint.Y, 0.f);
	BB->SetValueAsVector(TEXT("PatrolLocation"), PatrolLocation);
	return EBTNodeResult::Succeeded;
}
