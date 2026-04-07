// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MonsterAttack.generated.h"
// 뭘 위한 구조체? AttackMemory? 그리고 FDelegateHandle용도? MonsterBase헤더에있는 델리게이트랑 무슨 연관?
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
	// unint16? 왜? 그리고 GetInstanceMemorySize는 무슨 용도?
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTTask_AttackMemory); }
	// 각 함수 의도랑 매개변수의 풀이
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,EBTNodeResult::Type TaskResult) override;
	
	private:
	void ClearDelegate(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};
