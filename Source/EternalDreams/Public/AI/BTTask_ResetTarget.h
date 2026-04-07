// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ResetTarget.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UBTTask_ResetTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ResetTarget();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
