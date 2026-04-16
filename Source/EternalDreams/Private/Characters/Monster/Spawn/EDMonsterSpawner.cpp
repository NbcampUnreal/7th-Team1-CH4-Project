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
	UEDMonsterSpawnSubsystem* Subsystem = GetWorld()->GetSubsystem<UEDMonsterSpawnSubsystem>();
	if (IsValid(Subsystem) == false)
		return;
	// 서브 시스템 스포너 등록
	Subsystem->RegisterSpawner(this);
	// Normal은 게임 시작과 동시에 스폰 Elite/Boss는 트리거 대기
	
	UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(this);
	if (!DataSubsystem) return;

	if (DataSubsystem->IsDataReady())
	{
		// 데이터 로드되어있으면 바로 스폰
		SpawnMonster();
	}
	else
	{
		// 데이터 로딩중이면 스폰 체크함수 바인딩해놓음
		DataSubsystem->OnAllDataLoaded.AddDynamic(this, &AEDMonsterSpawner::OnDataLoadedResponse);
	}
}
void AEDMonsterSpawner::OnDataLoadedResponse()
{
	UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(this);
	if (!DataSubsystem) return;

	// 로드 끝나고 들어왔으므로 정상적으로 DataAsset포인터 반환
	UEDMonsterDataAsset* DataAsset = DataSubsystem->GetData<UEDMonsterDataAsset>(MonsterDataAssetId);
    
	// Normal은 게임 시작과 동시에 스폰 Elite/Boss는 트리거 대기
	if (IsValid(DataAsset) && DataAsset->GetGrade() == EMonsterGrade::Normal)
	{
		SpawnMonster();
	}
}

void AEDMonsterSpawner::SpawnMonster()
{
	UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(this);
	if (!DataSubsystem || !MonsterDataAssetId.IsValid()) return;
	UEDMonsterDataAsset* MonsterDataAsset = DataSubsystem->GetData<UEDMonsterDataAsset>(MonsterDataAssetId); //
	
	if (!IsValid(MonsterDataAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnMonster: 캐시된 DataAsset을 찾을 수 없음"), *GetName());
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
	UEDMonsterSpawnSubsystem* Subsystem = GetWorld()->GetSubsystem<UEDMonsterSpawnSubsystem>();
	if (IsValid(Subsystem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Subsystem: 등록 실패"), *GetName());
		return;
	}
	Subsystem->RegisterMonster(Monster);
	
	// 사망 콜백 바인딩- 리스폰 처리용
	// TODO: 몬스터 사망 델리게이트 연결 예정
}

void AEDMonsterSpawner::OnMonsterDeath()
{
	// 서브시스템에 사망 통보
	UEDMonsterSpawnSubsystem* Subsystem = GetWorld()->GetSubsystem<UEDMonsterSpawnSubsystem>();
	if (IsValid(Subsystem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Subsystem: 해제 실패"), *GetName());
		return;
	}
	Subsystem->OnMonsterDeath(SpawnedMonster);
	SpawnedMonster = nullptr;
	
	// RespawnDelay가 0이면 리스폰 없음 (BOSS)
	if (RespawnDelay <= 0.f)
		return;
	
	// 리스폰 타이머 설정
	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this, &AEDMonsterSpawner::SpawnMonster,
		RespawnDelay, false);
}
