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
#include "Engine/AssetManager.h"
#include "Chaos/Deformable/ChaosDeformableSolverProxy.h"


// Sets default values
AEDMonsterAIController::AEDMonsterAIController()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Perception
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);
	// 기본값 - OnPossess에서 DA의 DetectRange로 덮어씀
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
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] OnPossess: DataAsset 없음"), *GetName());
		return;
	}
	// DA의 DetectRange로 감지 범위 설정
	const float DetectRange = DA->GetStat().DetectRange;
	
	SightConfig->SightRadius = DetectRange;
	SightConfig->LoseSightRadius = DetectRange * 1.2f;
	HearingConfig->HearingRange = DetectRange * 0.8f;
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	// BT 비동기 로드(임시)
	if (DA->GetBehaviorTree().IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] BehaviorTree 레퍼런스 없음"), *GetName());
		return;
	}
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	BTLoadHandle = Streamable.RequestAsyncLoad(
		DA->GetBehaviorTree().ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &AEDMonsterAIController::OnBTLoaded)
		);
}

void AEDMonsterAIController::OnUnPossess()
{
	Super::OnUnPossess();
	GetWorldTimerManager().ClearTimer(TeamReportTimerHandle);
	
	if (BTLoadHandle.IsValid())
	{
		BTLoadHandle->CancelHandle();
		BTLoadHandle.Reset();
	}
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
			
			UBlackboardComponent* BB = GetBlackboardComponent();
			if (IsValid(BB) == false)
				continue;
			
			if (SenseClass == UAISense_Sight::StaticClass())
			{
				if (IsEliteOrBoss() == false)
					continue;
				UE_LOG(LogTemp, Warning, TEXT("[%s] Sight 감지(Elite/Boss): %s"), *GetName(), *Actor->GetName());
				BB->SetValueAsObject(TEXT("TargetActor"), Actor);
				StartTeamReport(Actor);
			}
			else if (SenseClass == UAISense_Damage::StaticClass() ||
					 SenseClass == UAISense_Touch::StaticClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] 반격 감지(%s): %s"), *GetName(), *SenseClass->GetName(), *Actor->GetName());
				BB->SetValueAsObject(TEXT("TargetActor"), Actor);
				StartTeamReport(Actor);
			}
			// Hearing 감지
			else if (SenseClass == UAISense_Hearing::StaticClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Hearing: %s 소리 감지"), *GetName(), *Actor->GetName());
				// TODO: BB_Monster 생성 후 활성화
				BB->SetValueAsVector(TEXT("LastHearingLocation"), Actor->GetActorLocation());
				BB->SetValueAsBool(TEXT("bIsTracking"), true);
			}
			// Team -> 아군으로부터 타겟 수신
			else if (SenseClass == UAISense_Team::StaticClass())
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Team: %s 정보 수신"), *GetName(), *Actor->GetName());
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
	BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
	BB->SetValueAsVector(TEXT("LastHearingLocation"), FVector::ZeroVector);
	BB->SetValueAsBool(TEXT("bIsTracking"), false);
}

bool AEDMonsterAIController::IsEliteOrBoss() const
{
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(GetPawn());
	if (IsValid(Monster) == false || IsValid(Monster->GetDataAsset()) == false)
		return false;
	
	EMonsterGrade Grade = Monster->GetDataAsset()->GetGrade();
	return Grade == EMonsterGrade::Elite || Grade == EMonsterGrade::Boss;
}

void AEDMonsterAIController::BroadcastTeamSense()
{
	if (TeamReportTarget.IsValid() == false)
	{
		GetWorldTimerManager().ClearTimer(TeamReportTimerHandle);
		return;
	}
	
	UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld());
	if (IsValid(PerceptionSystem) == false)
		return;
	
	FAITeamStimulusEvent Event = FAITeamStimulusEvent(
		this, TeamReportTarget.Get(), TeamReportTarget->GetActorLocation(), 1000.f);
	PerceptionSystem->OnEvent(Event);
}

void AEDMonsterAIController::StartTeamReport(AActor* Target)
{
	TeamReportTarget = Target;
	GetWorld()->GetTimerManager().SetTimer(
		TeamReportTimerHandle,
		this, &AEDMonsterAIController::BroadcastTeamSense,
		2.f, true, 0.5f);
}

void AEDMonsterAIController::OnBTLoaded()
{
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(GetPawn());
	if (IsValid(Monster) == false || IsValid(Monster->GetDataAsset()) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AICtrl][%s] OnBTLoaded: Pawn 또는 DataAsset 없음"), *GetName());
		return;
	}
	
	UBehaviorTree* BT = Monster->GetDataAsset()->GetBehaviorTree().Get();
	if (IsValid(BT) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AICtrl][%s] OnBTLoaded: BT 유효하지 않음"), *GetName());
		return;
	}
	
	bool bResult = RunBehaviorTree(BT);
	UE_LOG(LogTemp, Warning, TEXT("[AICtrl][%s] RunBehaviorTree(Async): %s"), *GetName(), bResult ? TEXT("성공") : TEXT("실패"));
}


