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

struct FStreamableHandle;
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
	// AI Perception
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	UFUNCTION()
	void OnPerceptionForgotten(AActor* Actor);
	
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
	// 몬스터 Grade를 기준으로 선공몬스터와 비선공 몬스터 구분
	bool IsEliteOrBoss() const;
	// 주변 몬스터에게 전투 상황 전달 (타이머 콜백)
	void BroadcastTeamSense();
	// 타겟 감지 시 팀 센스 브로드캐스트 타이머 시작
	void StartTeamReport(AActor* Target);
	// BT 비동기 로딩(임시)
	void OnBTLoaded();
	
	FGenericTeamId MonsterTeamId;
	FTimerHandle TeamReportTimerHandle;
	TWeakObjectPtr<AActor> TeamReportTarget;
	
	TSharedPtr<FStreamableHandle> BTLoadHandle;
};
