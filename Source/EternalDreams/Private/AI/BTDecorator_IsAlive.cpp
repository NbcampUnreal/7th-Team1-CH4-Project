// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_IsAlive.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"

UBTDecorator_IsAlive::UBTDecorator_IsAlive()
{
	NodeName = TEXT("Is Alive");
}

bool UBTDecorator_IsAlive::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
		return false;
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false)
		return false;
	
	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return false;
	
	// HP가 0 보다 크면 true 반환
	return ASC->GetNumericAttribute(UEDBaseAttributeSet::GetHealthAttribute()) > 0.f;
}
