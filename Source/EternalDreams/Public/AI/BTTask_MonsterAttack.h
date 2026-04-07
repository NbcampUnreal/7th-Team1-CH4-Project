// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MonsterAttack.generated.h"

struct FBTTask_AttackMemory
{
	FDelegateHandle OnAttackFinishedHandle;
};

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UBTTask_MonsterAttack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_MonsterAttack();
	
protected:
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTTask_AttackMemory); }
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,EBTNodeResult::Type TaskResult) override;
	
	private:
	void ClearDelegate(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};
