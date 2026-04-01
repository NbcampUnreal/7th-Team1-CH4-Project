// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include  "MonsterTypes.Generated.h"

/**
*
*/
// 몬스터 상태
UENUM(BlueprintType)
enum class EMonsterState : uint8
{
 Idle,
 Chase,
 Attack,
 Hit,
 Dead,
};
// 몬스터 타입
UENUM(BlueprintType)
enum class EMonsterGrade : uint8
{
 Normal,
 Elite,
 Boss,
};
// 인식 타입
UENUM(BlueprintType)
enum class EMonsterPerceptionType : uint8
{
 Sight,
 Damage,
 Touch,
 Hearing,
 Team,
};
// 스탯 구조체
USTRUCT(BlueprintType)
struct FMonsterStatRow
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 int32 MaxHP;

 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 int32 Def;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 int32 Atk;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 int32 MoveSpeed;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 int32 DetectRange;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 int32 AtkRange;
};


