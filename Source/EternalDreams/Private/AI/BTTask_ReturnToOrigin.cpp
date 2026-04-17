// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_ReturnToOrigin.h"
#include "AIController.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_ReturnToOrigin::UBTTask_ReturnToOrigin()
{
	NodeName = TEXT("Return To Origin");
}

EBTNodeResult::Type UBTTask_ReturnToOrigin::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
		return EBTNodeResult::Failed;
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false)
		return EBTNodeResult::Failed;
	
	// 스폰 위치로 이동
	EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(Monster->GetOriginLocation());
	if (Result == EPathFollowingRequestResult::Type::Failed)
		return EBTNodeResult::Failed;
	
	return EBTNodeResult::Succeeded;
}
