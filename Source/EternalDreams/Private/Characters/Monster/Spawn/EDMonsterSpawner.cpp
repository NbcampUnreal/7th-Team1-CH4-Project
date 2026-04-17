// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/Spawn/EDMonsterSpawner.h"
#include "Characters/Monster/Spawn/EDMonsterSpawnSubsystem.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Data/EDMonsterDataAsset.h"

// Sets default values
AEDMonsterSpawner::AEDMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AEDMonsterSpawner::TriggerSpawn()
{
	SpawnMonster();
}

// Called when the game starts or when spawned
void AEDMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() == false)
		return;
	
	UEDMonsterSpawnSubsystem* Subsystem = GetWorld()->GetSubsystem<UEDMonsterSpawnSubsystem>();
	if (IsValid(Subsystem) == false)
		return;
	
	// Subsystem 캐싱
	CachedSubsystem = Subsystem;

	// Normal은 게임 시작과 동시에 스폰 Elite/Boss는 트리거 대기
	if (IsValid(MonsterDataAsset) == false)
		return;
	
	// 서브 시스템 스포너 등록(Grade랑 같이)
	Subsystem->RegisterSpawner(this, MonsterDataAsset->GetGrade());
	// Normal만 스폰
	if (MonsterDataAsset->GetGrade() != EMonsterGrade::Normal)
		return;
	SpawnMonster();
}

void AEDMonsterSpawner::SpawnMonster()
{
	if (IsValid(MonsterDataAsset) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnMonster: DataAsset 없음"), *GetName());
		return;
	}
	
	TSubclassOf<AEDMonsterBase> SpawnClass = MonsterDataAsset->GetMonsterClass();
	if (IsValid(SpawnClass) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnMonster: MonsterClass 없음"), *GetName());
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.bDeferConstruction = true;
	
	AEDMonsterBase* Monster = GetWorld()->SpawnActor<AEDMonsterBase>(
		SpawnClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParams);
	
	if (IsValid(Monster) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnMonster: 스폰 실패"), *GetName());
		return;
	}
	
	// DA 적용
	Monster->InitializeFromDataAsset(MonsterDataAsset);
	Monster->FinishSpawning(FTransform(GetActorRotation(), GetActorLocation()));
		
	SpawnedMonster = Monster;
	// 서브시스템에 등록
	UEDMonsterSpawnSubsystem* Subsystem = CachedSubsystem.Get();
	if (IsValid(Subsystem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Subsystem: 등록 실패"), *GetName());
		return;
	}
	Subsystem->RegisterMonster(Monster);
	// 사망 콜백 바인딩- 리스폰 처리용
	Monster->OnMonsterDeath.AddUObject(this, &AEDMonsterSpawner::OnMonsterDeath);
}

void AEDMonsterSpawner::OnMonsterDeath()
{
	// 서브시스템에 사망 통보
	UEDMonsterSpawnSubsystem* Subsystem = CachedSubsystem.Get();
	if (IsValid(Subsystem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Subsystem: 해제 실패"), *GetName());
		return;
	}
	Subsystem->OnMonsterDeath(SpawnedMonster.Get());
	SpawnedMonster.Reset();
	
	// RespawnDelay가 0이면 리스폰 없음 (BOSS)
	if (RespawnDelay <= 0.f)
		return;
	
	// 리스폰 타이머 설정
	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this, &AEDMonsterSpawner::SpawnMonster,
		RespawnDelay, false);
}
