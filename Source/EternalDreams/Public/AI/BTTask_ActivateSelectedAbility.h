// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ActivateSelectedAbility.generated.h"

/**
 * 
 */
struct FBTTask_ActivatedAbilityMemory
{
	FDelegateHandle OnAttackFinishedHandle;
};

UCLASS()
class ETERNALDREAMS_API UBTTask_ActivateSelectedAbility : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ActivateSelectedAbility();
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTTask_ActivatedAbilityMemory); }
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
private:
	void ClearDelegate(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	
};
