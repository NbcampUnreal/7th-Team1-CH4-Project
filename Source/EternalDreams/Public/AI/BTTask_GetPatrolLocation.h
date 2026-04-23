// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetPatrolLocation.generated.h"

class AEDRestrictedArea;

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UBTTask_GetPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_GetPatrolLocation();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	float PatrolRadius = 500.f;
	
private:
	FVector FindSafePatrolOrigin(UWorld* World, const FVector& CurrentOrigin) const;
};
