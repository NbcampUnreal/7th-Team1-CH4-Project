// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_MonsterAttack.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Monster/EDMonsterBase.h"

UBTTask_MonsterAttack::UBTTask_MonsterAttack()
{
	NodeName = TEXT("Monster Attack");
}

EBTNodeResult::Type UBTTask_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTTask_AttackMemory* Memory = CastInstanceNodeMemory<FBTTask_AttackMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (Memory == nullptr || IsValid(AIController) == false)
		return EBTNodeResult::Failed;
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false)
		return EBTNodeResult::Failed;
	
	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (IsValid(BB) == false)
		return EBTNodeResult::Failed;
	
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (IsValid(TargetActor) == false)
		return EBTNodeResult::Failed;
	// 공격 어빌리티 활성화 시도
	bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("Ability.Monster.Attack"))));
	if (bActivated == false)
		return EBTNodeResult::Failed;
	
	AIController->SetFocus(TargetActor);
	
	Monster->OnAttackFinished.Remove(Memory->OnAttackFinishedHandle);
	
	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = &OwnerComp;
	TWeakObjectPtr<UBTTask_MonsterAttack> WeakThis = this;
	
	Memory->OnAttackFinishedHandle = Monster->OnAttackFinished.AddLambda(
		[WeakThis, WeakOwnerComp]()
		{
			if (WeakOwnerComp.IsValid() == false || WeakThis.IsValid() == false)
				return;
			
			WeakThis->FinishLatentTask(*WeakOwnerComp, EBTNodeResult::Succeeded);
		}
	);
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_MonsterAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ClearDelegate(OwnerComp, NodeMemory);
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_MonsterAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	ClearDelegate(OwnerComp, NodeMemory);
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_MonsterAttack::ClearDelegate(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTTask_AttackMemory* Memory = CastInstanceNodeMemory<FBTTask_AttackMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
		return;
	
	AIController->ClearFocus(EAIFocusPriority::Gameplay);
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false || Memory == nullptr)
		return;
	if (Memory->OnAttackFinishedHandle.IsValid() == false)
		return;
	
	Monster->OnAttackFinished.Remove(Memory->OnAttackFinishedHandle);
	Memory->OnAttackFinishedHandle.Reset();
}
