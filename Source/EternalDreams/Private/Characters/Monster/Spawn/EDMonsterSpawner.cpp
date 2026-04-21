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

UEDMonsterDataAsset* AEDMonsterSpawner::GetMonsterDataAsset() const
{
	UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(this);
	if (!IsValid(DataSubsystem))
		return nullptr;

	return DataSubsystem->GetData<UEDMonsterDataAsset>(MonsterDataAssetId);
}

// Called when the game starts or when spawned
void AEDMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() == false)
		return;

	UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(this);
	if (!IsValid(DataSubsystem)) return;
	
	if (DataSubsystem->IsDataReady())
	{
		OnDataLoadedResponse();
	} else
	{
		DataSubsystem->OnAllDataLoaded.AddDynamic(this, &AEDMonsterSpawner::OnDataLoadedResponse);
	}
}


void AEDMonsterSpawner::OnDataLoadedResponse()
{
	if (bIsDataReady) return;
	bIsDataReady = true;

	UEDMonsterSpawnSubsystem* Subsystem = GetWorld()->GetSubsystem<UEDMonsterSpawnSubsystem>();
	if (IsValid(Subsystem) == false) return;
	
	CachedSubsystem = Subsystem;
	
	UEDMonsterDataAsset* MonsterDataAsset = GetMonsterDataAsset();
	if (!MonsterDataAsset) return;

	// 서브 시스템 스포너 등록(Grade랑 같이)
	Subsystem->RegisterSpawner(this, MonsterDataAsset->GetGrade());
	
	// Normal만 스폰
	if (MonsterDataAsset->GetGrade() != EMonsterGrade::Normal) return;
	
	SpawnMonster();
}

void AEDMonsterSpawner::SpawnMonster()
{
	// 데이터 로드가 안 되었으면 스폰 차단
	if (!bIsDataReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnMonster: 데이터 로드가 아직 완료되지 않았습니다."), *GetName());
		return;
	}
	
	UEDMonsterDataAsset* MonsterDataAsset = GetMonsterDataAsset();
	if (IsValid(MonsterDataAsset) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnMonster: DataAsset 없음"), *GetName());
		return;
	}
	TSubclassOf<AEDMonsterBase> SpawnClass = MonsterDataAsset->GetMonsterClass().Get();
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
	Monster->FinishSpawning(FTransform(GetActorRotation(), GetActorLocation()));
	Monster->InitializeFromDataAsset(MonsterDataAsset);
		
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

