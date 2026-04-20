// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Data/Types/EDMonsterAbilityScore.h"
#include "BTService_SelectAbility.generated.h"

class AEDMonsterBase;

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UBTService_SelectAbility : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_SelectAbility();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// Ability 선택 기준 구조체
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Score")
	TArray<FAbilityScoreContext> AbilityScoreList;
	
private:
	FGameplayTag SelectBestAbility(AEDMonsterBase* Monster, AActor* Target) const;

};
