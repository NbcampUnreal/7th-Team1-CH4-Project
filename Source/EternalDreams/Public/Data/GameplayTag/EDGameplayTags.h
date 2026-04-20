// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* [Native GameplayTag Guide]
 * 게임플레이 태그를 싱글톤 클래스로 관리합니다.
 * 
 *  사용 예시
 *  #include "Data/GameplayTag/EDGameplayTags.h"
 *  FEDGameplayTags::Get().Ability_Player_BasicAttack
 *   
 * 기존 방식 
 * ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Player.BasicAttack"));
 * 변경 코드
 * ActivationBlockedTags.AddTag(FEDGameplayTags::Get().Ability_Player_BasicAttack);
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ETERNALDREAMS_API FEDGameplayTags
{
public:
	// Singleton 접근자
	static const FEDGameplayTags& Get();

	// 초기화
	static void InitializeNativeTags();
	//Ability_Monster
	FGameplayTag Ability_Monster_Attack;
	FGameplayTag Ability_Monster_Skill;
	//Player_BasicAttack
	FGameplayTag Player_BasicAttack_Hammer;
	FGameplayTag Player_BasicAttack_Sword;
	FGameplayTag Player_BasicAttack_Staff;
	FGameplayTag Player_BasicAttack_Bow;
	//Player_Evade
	FGameplayTag Player_Evade_Hammer;
	FGameplayTag Player_Evade_Sword;
	FGameplayTag Player_Evade_Staff;
	FGameplayTag Player_Evade_Bow;
	//Player_Skill
	FGameplayTag Player_Skill_Whirlwind;
	FGameplayTag Player_Skill_Whirllaser;
	FGameplayTag Player_Skill_BasicKnockBack;
	//CoolDown
	FGameplayTag CoolDown_Evade_Hammer;
	FGameplayTag CoolDown_Evade_Sword;
	FGameplayTag CoolDown_Evade_Staff;
	FGameplayTag CoolDown_Evade_Bow;
	FGameplayTag CoolDown_Skill_Whirlwind;
	FGameplayTag CoolDown_Skill_Whirllaser;
	FGameplayTag CoolDown_Skill_BasicKnockBack;
	//Effect_Debuff
	FGameplayTag Effect_Debuff_Slow;
	FGameplayTag Effect_Debuff_Stun;
	FGameplayTag Effect_Debuff_Poison;
	//Effect_Buff
	FGameplayTag Effect_Buff_AttackUp;
	FGameplayTag Effect_Buff_Shield;
	//State
	FGameplayTag State_Player_CannotCanceled;
	FGameplayTag State_Player_Invincible;
	FGameplayTag State_Player_Unstoppable;
	FGameplayTag State_Player_Stop;
	
	FGameplayTag State_Player_RestrictedArea;
	FGameplayTag State_Debuff_RestrictedArea;
	// State_Common
	FGameplayTag State_Dead;
	//Phase
	FGameplayTag Phase_Day1_Day;
	FGameplayTag Phase_Day1_Night;
	FGameplayTag Phase_Day2_Day;
	FGameplayTag Phase_Day2_Night;
	FGameplayTag Phase_Day3_Day;
	FGameplayTag Phase_Day3_Night;
	FGameplayTag Phase_Day4_Day;
	FGameplayTag Phase_Day4_Night;
	//Combat
	FGameplayTag Combat_Window_Combo;
	FGameplayTag Combat_Window_Parry;
	//Data
	FGameplayTag Data_Damage;
	FGameplayTag Data_StaminaCost;
	FGameplayTag Data_CoolTime;
	FGameplayTag Data_StatAdd;
	FGameplayTag Data_StatAdd_Strength;
	FGameplayTag Data_StatAdd_Dexterity;
	FGameplayTag Data_StatAdd_Intelligence;
	FGameplayTag Data_StatMul;
	//Event
	FGameplayTag Event_SkillHit;
	
	//Inventory-Item
	FGameplayTag Item_Weapon_Sword;
	FGameplayTag Item_Weapon_Hammer;
	FGameplayTag Item_Weapon_Bow;
	FGameplayTag Item_Weapon_Staff;
	
	FGameplayTag Item_Armor_TopArmor;
	FGameplayTag Item_Armor_BottomArmor;
	
	FGameplayTag Item_Consumable;
	FGameplayTag Item_Ingredient_Base;
	FGameplayTag Item_Ingredient_Special;


private:
	// Singleton 인스턴스
	static FEDGameplayTags Tags;
};