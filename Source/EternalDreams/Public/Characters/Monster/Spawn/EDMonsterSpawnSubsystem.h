// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/Types/EDMonsterTypes.h"
#include "EDMonsterSpawnSubsystem.generated.h"

class AEDMonsterBase;
class AEDMonsterSpawner;

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDMonsterSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// 몬스터 등록 - Spawner에서 스폰 시 호출
	void RegisterMonster(AEDMonsterBase* Monster);
	// 스포너 등록
	void RegisterSpawner(AEDMonsterSpawner* Spawner);
	//  Grade별 스폰 트리거 - GameState/GameMode에서 호출(Elite/Boss용)
	void TriggerSpawnByGrade(EMonsterGrade Grade);
	// 몬스터 사망 처리 - Spawner에서 호출
	void OnMonsterDeath(AEDMonsterBase* DeadMonster);
	
	// 현재 활성 몬스터 목록 반환
	const TArray<TWeakObjectPtr<AEDMonsterBase>>& GetActiveMonsters() const { return ActiveMonsters; }
	
private:
	// OnMonsterDeath 내부에서만 호출, 활성 몬스터 목록에서 해제
	void UnregisterMonster(AEDMonsterBase* Monster);
	// 활성 몬스터 목록
	TArray<TWeakObjectPtr<AEDMonsterBase>> ActiveMonsters;
	// 레벨에 배치된 스포너 목록
	TArray<TWeakObjectPtr<AEDMonsterSpawner>> ActiveSpawners;
};
