// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
// 플레이어 관련 PDA 카테고리
UENUM(BlueprintType)
enum class EPlayerDataType : uint8
{
	PlayerData			UMETA(DisplayName = "PlayerData"),			//플레이어 스킨
	PlayerAnimData		UMETA(DisplayName = "PlayerAnimData"),		//플레이어 애니메이션 몽타주	
	WeaponData			UMETA(DisplayName = "WeaponData")			//무기	
};

//플레이어 스킨 관련 DA 이름
UENUM(BlueprintType)
enum class EPlayerNameType : uint8
{
	Basic			UMETA(DisplayName = "DA_Basic"),		
	Barbarian		UMETA(DisplayName = "DA_Barbarian"),	
	Druid			UMETA(DisplayName = "DA_Druid"),
	Nekku			UMETA(DisplayName = "DA_Nekku"),	
	Elf				UMETA(DisplayName = "DA_Elf")
};


//플레이어 무기 관련 DA 이름
UENUM(BlueprintType)
enum class EWeaponNameType : uint8
{
	Hammer			UMETA(DisplayName = "DA_Hammer"),		
	Sword			UMETA(DisplayName = "DA_Sword"),			
	Bow				UMETA(DisplayName = "DA_Bow"),			
	Staff			UMETA(DisplayName = "DA_Staff"),
	Arrow			UMETA(DisplayName = "DA_Arrow")
};

//플레이어 애님몽타주 관련 DA 이름
UENUM(BlueprintType)
enum class EPlayerAnimNameType : uint8
{
	// Basic Attack
	Player_BasicAttack_Hammer      UMETA(DisplayName = "DA_BasicAttack_Hammer"),
	Player_BasicAttack_Sword       UMETA(DisplayName = "DA_BasicAttack_Sword"),
	Player_BasicAttack_Staff       UMETA(DisplayName = "DA_BasicAttack_Staff"),
	Player_BasicAttack_Bow         UMETA(DisplayName = "DA_BasicAttack_Bow"),

	// Evade(Bow는 별도 몽타주 없음)
	Player_Evade_Hammer            UMETA(DisplayName = "DA_Evade_Hammer"),
	Player_Evade_Sword             UMETA(DisplayName = "DA_Evade_Sword"),
	Player_Evade_Staff             UMETA(DisplayName = "DA_Evade_Staff"),

	// Skill
	Player_Skill_Whirlwind         UMETA(DisplayName = "DA_Skill_Whirlwind"),
	Player_Skill_Whirllaser        UMETA(DisplayName = "DA_Skill_Whirllaser"),
	Player_Skill_BasicKnockBack    UMETA(DisplayName = "DA_Skill_BasicKnockBack"),
	Player_Skill_BasicStun		   UMETA(DisplayName = "DA_Skill_BasicStun"),
	
	//State
	State_Player_Stun				UMETA(DisplayName = "DA_Status_Stun"),
	NONE						   
};

