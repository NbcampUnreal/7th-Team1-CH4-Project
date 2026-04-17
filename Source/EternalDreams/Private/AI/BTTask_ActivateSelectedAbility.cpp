// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_ActivateSelectedAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Monster/EDMonsterBase.h"

UBTTask_ActivateSelectedAbility::UBTTask_ActivateSelectedAbility()
{
	NodeName = TEXT("Activate Selected Ability");
}

EBTNodeResult::Type UBTTask_ActivateSelectedAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTTask_ActivatedAbilityMemory* Memory = CastInstanceNodeMemory<FBTTask_ActivatedAbilityMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (Memory == nullptr || AIController == nullptr)
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
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (IsValid(Target) == false)
		return EBTNodeResult::Failed;
	
	// BB에서 선택된 어빌리티 태그
	FName TagName = BB->GetValueAsName(TEXT("SelectedAbilityTag"));
	if (TagName.IsNone())
		return EBTNodeResult::Failed;
	// false: 미등록 태그면 크래시 대신 InValid 반환
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TagName, false);
	if (Tag.IsValid() == false)
		return EBTNodeResult::Failed;
	
	bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(Tag));
	UE_LOG(LogTemp, Warning, TEXT("[ActivateSelectedAbility] Tag: %s → %s"),
		*TagName.ToString(), bActivated ? TEXT("성공") : TEXT("실패"));
	
	if (bActivated == false)
		return EBTNodeResult::Failed;
	

	
	AIController->SetFocus(Target);
	// 이전 실행에서 핸들이 남아 있을 수 있어 중복 등록 방지
	Monster->OnAttackFinished.Remove(Memory->OnAttackFinishedHandle);
	
	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = &OwnerComp;
	TWeakObjectPtr<UBTTask_ActivateSelectedAbility> WeakThis = this;
	// OnAttackFinishedHandle에 저장
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

EBTNodeResult::Type UBTTask_ActivateSelectedAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ClearDelegate(OwnerComp, NodeMemory);
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ActivateSelectedAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	ClearDelegate(OwnerComp, NodeMemory);
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_ActivateSelectedAbility::ClearDelegate(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTTask_ActivatedAbilityMemory* Memory = CastInstanceNodeMemory<FBTTask_ActivatedAbilityMemory>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (Memory == nullptr || AIController == nullptr)
		return;
	// AIController의 포커스를 정리
	AIController->ClearFocus(EAIFocusPriority::Gameplay);
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false)
		return;
	// OnAttackFinishedHandle에 핸들이 남아있다면 제거
	if (Memory->OnAttackFinishedHandle.IsValid() == false)
		return;
	Monster->OnAttackFinished.Remove(Memory->OnAttackFinishedHandle);
	Memory->OnAttackFinishedHandle.Reset();
}

