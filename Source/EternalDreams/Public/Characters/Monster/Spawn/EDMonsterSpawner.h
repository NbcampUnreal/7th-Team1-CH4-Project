// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/EDGameDataSubsystem.h"
#include "EDMonsterSpawner.generated.h"

class UEDMonsterDataAsset;
class AEDMonsterBase;
class UEDMonsterSpawnSubsystem;

UCLASS()
class ETERNALDREAMS_API AEDMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	AEDMonsterSpawner();
	
	// Subsystem에서 Grade별 트리거 시 호출(Elite/Boss용)
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void TriggerSpawn();
	
	UEDMonsterDataAsset* GetMonsterDataAsset() const;
protected:
	virtual void BeginPlay() override;
	
	// 스폰할 몬스터 DA의 에셋 Id
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FPrimaryAssetId MonsterDataAssetId;
	
	// 리스폰 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float RespawnDelay = 30.f;

private:
	// 몬스터 스폰
	void SpawnMonster();
	// 몬스터 사망 시 콜백
	void OnMonsterDeath();
	
	// 현재 스폰된 몬스터
	TWeakObjectPtr<AEDMonsterBase> SpawnedMonster;
	// WorldSubsystem 캐싱 - BeginPlay에서 한 번만 조회
	TWeakObjectPtr<UEDMonsterSpawnSubsystem> CachedSubsystem;
	
	FTimerHandle RespawnTimerHandle;
	
	UFUNCTION()
	void OnDataLoadedResponse();
};
