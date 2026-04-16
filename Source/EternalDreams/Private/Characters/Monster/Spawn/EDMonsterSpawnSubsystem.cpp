// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/Spawn/EDMonsterSpawnSubsystem.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Characters/Monster/Spawn/EDMonsterSpawner.h"

void UEDMonsterSpawnSubsystem::RegisterMonster(AEDMonsterBase* Monster)
{
	if (IsValid(Monster) == false)
		return;
	// 중복 등록 방지
	for (const TWeakObjectPtr<AEDMonsterBase>& Existing : ActiveMonsters)
	{
		if (Existing.Get() == Monster)
			return;
	}
	ActiveMonsters.Add(Monster);
}

void UEDMonsterSpawnSubsystem::RegisterSpawner(AEDMonsterSpawner* Spawner, EMonsterGrade Grade)
{
	if (IsValid(Spawner) == false)
		return;
	// 중복 등록 방지
	TArray<TWeakObjectPtr<AEDMonsterSpawner>>& List = SpawnersByGrade.FindOrAdd(Grade);
	for (const TWeakObjectPtr<AEDMonsterSpawner>& Existing : List)
	{
		if (Existing.Get() == Spawner)
			return;
	}
	
	List.Add(Spawner);
}

void UEDMonsterSpawnSubsystem::TriggerSpawnByGrade(EMonsterGrade Grade)
{
	TArray<TWeakObjectPtr<AEDMonsterSpawner>>* List = SpawnersByGrade.Find(Grade);
	if (List == nullptr)
		return;
	for (const TWeakObjectPtr<AEDMonsterSpawner>& Weak : *List)
	{
		if (AEDMonsterSpawner* Spawner = Weak.Get())
			Spawner->TriggerSpawn();
	}
}

void UEDMonsterSpawnSubsystem::OnMonsterDeath(AEDMonsterBase* DeadMonster)
{
	UnregisterMonster(DeadMonster);
	// TODO: GameState 연동 - 아이템 드랍, 경험치 처리 등
}

void UEDMonsterSpawnSubsystem::UnregisterMonster(AEDMonsterBase* Monster)
{
	if (IsValid(Monster) == false)
		return;
	// 사망 해제된 몬스터 제거
	ActiveMonsters.RemoveAll([Monster](const TWeakObjectPtr<AEDMonsterBase>& Weak)
	{
		return Weak.Get() == Monster;
	});
}
