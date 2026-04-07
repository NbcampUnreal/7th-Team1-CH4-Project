// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_ResetTarget.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ResetTarget::UBTTask_ResetTarget()
{
	NodeName = TEXT("Reset Target");
}

EBTNodeResult::Type UBTTask_ResetTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (IsValid(BB) == false)
		return EBTNodeResult::Failed;
	BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
	BB->SetValueAsVector(TEXT("LastHearingLocation"), FVector::ZeroVector);
	BB->SetValueAsBool(TEXT("bIsTracking"), false);
	
	return EBTNodeResult::Succeeded;
}
