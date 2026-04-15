// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/Spawn/EDMonsterSpawnSubsystem.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Characters/Monster/Spawn/EDMonsterSpawner.h"
#include "Data/EDMonsterDataAsset.h"

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

void UEDMonsterSpawnSubsystem::RegisterSpawner(AEDMonsterSpawner* Spawner)
{
	if (IsValid(Spawner) == false)
		return;
	
	// 중복 등록 방지
	for (const TWeakObjectPtr<AEDMonsterSpawner>& Existing : ActiveSpawners)
	{
		if (Existing.Get() == Spawner)
			return;
	}
	ActiveSpawners.Add(Spawner);
}

void UEDMonsterSpawnSubsystem::TriggerSpawnByGrade(EMonsterGrade Grade)
{
	for (const TWeakObjectPtr<AEDMonsterSpawner>& Weak : ActiveSpawners)
	{
		AEDMonsterSpawner* Spawner = Weak.Get();
		if (IsValid(Spawner) == false || IsValid(Spawner->GetMonsterDataAsset()) == false)
			continue;
		// 해당 Grade의 스포너만 트리거
		if (Spawner->GetMonsterDataAsset()->GetGrade() == Grade)
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
