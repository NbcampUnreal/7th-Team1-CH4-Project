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
	//Ability_Player
	FGameplayTag Ability_Player_BasicAttack;
	FGameplayTag Ability_Player_Skill;
	FGameplayTag Ability_Player_Buff;
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
;
	//Combat
	FGameplayTag Combat_Window_Combo;
	FGameplayTag Combat_Window_Parry;
	//Data
	FGameplayTag Data_Damage;
	FGameplayTag Data_StaminaCost;

private:
	// Singleton 인스턴스
	static FEDGameplayTags Tags;
};