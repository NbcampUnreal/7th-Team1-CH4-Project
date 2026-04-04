// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/EDMonsterAIController.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Data/EDMonsterDataAsset.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Touch.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Team.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"


// Sets default values
AEDMonsterAIController::AEDMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Perception
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);
	// magic number는 추후에 수정 
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = 1200.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies= true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageConfig->SetMaxAge(5.f);
	
	TouchConfig = CreateDefaultSubobject<UAISenseConfig_Touch>(TEXT("TouchConfig"));
	TouchConfig->SetMaxAge(5.f);
	TouchConfig->DetectionByAffiliation.bDetectEnemies = true;
	TouchConfig->DetectionByAffiliation.bDetectFriendlies = false;
	TouchConfig->DetectionByAffiliation.bDetectNeutrals = false;
	
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 800.f;
	HearingConfig->SetMaxAge(5.f);
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = false;
	
	TeamConfig = CreateDefaultSubobject<UAISenseConfig_Team>(TEXT("TeamConfig"));
	TeamConfig->SetMaxAge(5.f);
	
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*DamageConfig);
	AIPerceptionComp->ConfigureSense(*TouchConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*TeamConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AEDMonsterAIController::OnPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionForgotten.AddDynamic(this, &AEDMonsterAIController::OnPerceptionForgotten);
	
	MonsterTeamId = FGenericTeamId(1);
}

// Called when the game starts or when spawned
void AEDMonsterAIController::BeginPlay()
{
	Super::BeginPlay();
	
}

ETeamAttitude::Type AEDMonsterAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* OtherPawn = Cast<APawn>(&Other);
	if (IsValid(OtherPawn) == false)
		return ETeamAttitude::Neutral;
	
	const IGenericTeamAgentInterface* OtherTeam = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
	if (OtherTeam == nullptr)
		return ETeamAttitude::Neutral;
	
	return MonsterTeamId == OtherTeam->GetGenericTeamId() 
	? ETeamAttitude::Friendly 
	: ETeamAttitude::Hostile;
}
// TODO: 비동기 로드로 교체 추후에
void AEDMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (HasAuthority() == false)
		return;
	SetGenericTeamId(MonsterTeamId);
	UAIPerceptionSystem::GetCurrent(GetWorld())->UpdateListener(*AIPerceptionComp);
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(InPawn);
	if (IsValid(Monster) == false)
		return;
	
	UEDMonsterDataAsset* DA = Monster->GetDataAsset();
	if (IsValid(DA) == false)
		return;
	
	UBehaviorTree* BT = DA->GetBehaviorTree().LoadSynchronous();
	if (IsValid(BT) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to load BehaviorTree!"), *GetName());
		return;
	}
	
	RunBehaviorTree(BT);
}

void AEDMonsterAIController::OnUnPossess()
{
	Super::OnUnPossess();
	GetWorldTimerManager().ClearTimer(TeamReportTimerHandle);
}

void AEDMonsterAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (HasAuthority() == false)
		return;
	
	for (AActor* Actor : UpdatedActors)
	{
		if (IsValid(Actor) == false)
			continue;
		FActorPerceptionBlueprintInfo Info;
		AIPerceptionComp->GetActorsPerception(Actor, Info);
		
		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			if (Stimulus.WasSuccessfullySensed() == false)
				continue;
			TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);
			if (IsValid(SenseClass) == false)
				continue;
			if (SenseClass == UAISense_Sight::StaticClass() ||
				SenseClass == UAISense_Damage::StaticClass() ||
				SenseClass == UAISense_Touch::StaticClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] 감지: %s (%s)"), *GetName(), *Actor->GetName(), *SenseClass->GetName());
				
				UBlackboardComponent* BB = GetBlackboardComponent();
				if (IsValid(BB) == false)
					continue;
				
				BB->SetValueAsObject(TEXT("TargetActor"), Actor);
				
				// Ramda함수 안에서 안전하게 사용하기 위한 TWeakObjectPtr
				TWeakObjectPtr<AEDMonsterAIController> WeakThis(this);
				TWeakObjectPtr<AActor> WeakActor(Actor);
				
				// Team Sense로 주변 아군에게 타겟 공유
				GetWorld()->GetTimerManager().SetTimer(TeamReportTimerHandle,
					[WeakThis, WeakActor]()
					{
						if (WeakThis.IsValid() == false)
							return;
						
						UWorld* World = WeakThis->GetWorld();
						if (IsValid(World) == false)
							return;
						
						if (WeakActor.IsValid() == false)
						{
							WeakThis->GetWorldTimerManager().ClearTimer(WeakThis->TeamReportTimerHandle);
							return;
						}
						
						UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(World);
						if (IsValid(PerceptionSystem) == false)
							return;
						
						FAITeamStimulusEvent Event = FAITeamStimulusEvent(WeakThis.Get(), WeakActor.Get(), WeakActor->GetActorLocation(), 1000.f);
						PerceptionSystem->OnEvent(Event);
					},
					2.f, true, 0.5f);
			}
			// Hearing 감지
			else if (SenseClass == UAISense_Hearing::StaticClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Hearing: %s 소리 감지"), *GetName(), *Actor->GetName());
				// TODO: BB_Monster 생성 후 활성화
				UBlackboardComponent* BB = GetBlackboardComponent();
				if (IsValid(BB) == false)
					continue;
				BB->SetValueAsVector(TEXT("LastHearingLocation"), Actor->GetActorLocation());
				BB->SetValueAsBool(TEXT("bIsTracking"), true);
			}
			// Team -> 아군으로부터 타겟 수신
			else if (SenseClass == UAISense_Team::StaticClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Team: %s 정보 수신"), *GetName(), *Actor->GetName());
				UBlackboardComponent* BB = GetBlackboardComponent();
				if (IsValid(BB) == false)
					continue;
				if (IsValid(BB->GetValueAsObject(TEXT("TargetActor"))) == false)
					BB->SetValueAsObject(TEXT("TargetActor"), Actor);
			}
		}
	}
}

void AEDMonsterAIController::OnPerceptionForgotten(AActor* Actor)
{
	if (IsValid(Actor) == false)
		return;
	UE_LOG(LogTemp, Warning, TEXT("[%s] 잊혀진 액터: %s"), *GetName(),*Actor->GetName());
	
	GetWorldTimerManager().ClearTimer(TeamReportTimerHandle);
	
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (IsValid(BB) == false)
		return;
	BB->SetValueAsVector(TEXT("LastHearingLocation"), FVector::ZeroVector);
	BB->SetValueAsBool(TEXT("bIsTracking"), false);
}


