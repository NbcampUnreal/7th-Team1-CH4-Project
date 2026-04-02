// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EDMonsterTypes.Generated.h"

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
 float MaxHP;

 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 float Def;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 float Atk;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 float MoveSpeed;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 float DetectRange;
 
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
 float AtkRange;
};


