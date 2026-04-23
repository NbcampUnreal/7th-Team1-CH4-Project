// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 */

// 플레이어 관련 PDA 카테고리
UENUM(BlueprintType)
enum class EPlayerDataType : uint8
{
	PlayerData,       // 플레이어 스킨
	PlayerAnimData,    // 플레이어 애니메이션 몽타주   
	WeaponData        // 무기   
};

// 플레이어 스킨 관련 DA 이름
UENUM(BlueprintType)
enum class EPlayerNameType : uint8
{
	DA_Basic,      
	DA_Barbarian,   
	DA_Druid,
	DA_Nekku,   
	DA_Elf
};

// 플레이어 무기 관련 DA 이름
UENUM(BlueprintType)
enum class EWeaponNameType : uint8
{
	DA_Hammer,     
	DA_Sword,         
	DA_Bow,       
	DA_Staff,
	DA_Arrow
};

// 플레이어 애님몽타주 관련 DA 이름
UENUM(BlueprintType)
enum class EPlayerAnimNameType : uint8
{
	// Basic Attack
	DA_BasicAttack_Hammer,
	DA_BasicAttack_Sword,
	DA_BasicAttack_Staff,
	DA_BasicAttack_Bow,

	// Evade
	DA_Evade_Hammer,
	DA_Evade_Sword,
	DA_Evade_Staff,

	// Skill
	DA_Skill_Whirlwind,
	DA_Skill_Whirllaser,
	DA_Skill_BasicKnockBack,
	DA_Skill_BasicStun,
    
	// State
	DA_State_Stun,
    
	NONE                     
};