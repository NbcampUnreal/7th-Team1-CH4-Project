// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Touch.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Team.h"
#include "GenericTeamAgentInterface.h"
#include "EDMonsterAIController.generated.h"

class UBehaviorTree;

UCLASS()
class ETERNALDREAMS_API AEDMonsterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEDMonsterAIController();
	
	virtual FGenericTeamId GetGenericTeamId() const override { return MonsterTeamId; }
	
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	UFUNCTION()
	void OnPerceptionForgotten(AActor* Actor);
	// AI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|SenseConfig")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|SenseConfig")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|SenseConfig")
	TObjectPtr<UAISenseConfig_Touch> TouchConfig;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|SenseConfig")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|SenseConfig")
	TObjectPtr<UAISenseConfig_Team> TeamConfig;
	
private:
	FGenericTeamId MonsterTeamId;
	FTimerHandle TeamReportTimerHandle;
};
