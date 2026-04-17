// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_SelectAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"

UBTService_SelectAbility::UBTService_SelectAbility()
{
	NodeName = TEXT("Select Ability");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UBTService_SelectAbility::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
		return;
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false)
		return;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (IsValid(BB) == false)
		return;
	
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (IsValid(Target) == false)
		return;
	// GameplayTag Selected에 BaseAbility에서 스코어 별로 계산한 최선의 선택을 태그로 반환
	FGameplayTag Selected = SelectBestAbility(Monster, Target);
	// 블랙보드에서 Selected에 저장된 최선의 태그를 이용해 GA 사용 
	BB->SetValueAsName(TEXT("SelectedAbilityTag"), Selected.GetTagName());
}

FGameplayTag UBTService_SelectAbility::SelectBestAbility(AEDMonsterBase* Monster, AActor* Target) const
{
	if (AbilityScoreList.IsEmpty())
		return FGameplayTag::EmptyTag;
	
	UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return FGameplayTag::EmptyTag;
	
	float Distance = FVector::Dist(Monster->GetActorLocation(), Target->GetActorLocation());
	
	float HP = ASC->GetNumericAttribute(UEDBaseAttributeSet::GetHealthAttribute());
	float MaxHP = ASC->GetNumericAttribute(UEDBaseAttributeSet::GetMaxHealthAttribute());
	float HPRatio = (MaxHP > 0.f) ? (HP / MaxHP) : 1.f;
	
	float BestScore = 0.f;
	FGameplayTag BestTag = FGameplayTag::EmptyTag;
	for (const FAbilityScoreContext& Context : AbilityScoreList)
	{
		if (Context.AbilityTag.IsValid() == false)
			continue;
		// 쿨타임 중이면 스킵
		if (Context.CooldownTag.IsValid() && ASC->HasMatchingGameplayTag(Context.CooldownTag))
			continue;
		float Score = Context.BaseScore;
		// RangeType에 따라 거리 점수 계산
		if (Context.RangeType == EAbilityRangeType::Melee && Distance <= Context.PreferredRange)
			Score += Context.RangeScore;
		else if (Context.RangeType == EAbilityRangeType::Ranged && Distance >= Context.PreferredRange)
			Score += Context.RangeScore;
		// RageHPThreshold 미설정이면 Rage조건 스킵
		if (Context.RageHPThreshold > 0.f && HPRatio < Context.RageHPThreshold)
			Score += Context.RageScore;
		
		UE_LOG(LogTemp, Verbose, TEXT("[SelectAbility] %s → %.1f (Dist:%.1f HP:%.0f%%)"), 
			*Context.AbilityTag.ToString(), Score, Distance, HPRatio * 100.f);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTag = Context.AbilityTag;
		}
	}
	return BestTag;
}
