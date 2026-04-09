// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/GameplayTag/EDGameplayTags.h"
#include "GameplayTagsManager.h"

FEDGameplayTags FEDGameplayTags::Tags;

const FEDGameplayTags& FEDGameplayTags::Get()
{
	return Tags;
}

void FEDGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
	//Ability_Monster
	Tags.Ability_Monster_Attack = Manager.AddNativeGameplayTag(
		FName("Ability.Monster.Attack"), 
		FString("몬스터가 공격 중")
		);
	Tags.Ability_Monster_Skill = Manager.AddNativeGameplayTag(
		FName("Ability.Monster.Skill"), 
		FString("몬스터가 스킬 사용 중")
		);
	//Ability_Player
	Tags.Ability_Player_BasicAttack = Manager.AddNativeGameplayTag(
		FName("Ability.Player.BasicAttack"), 
		FString("플레이어가 기본 공격중")
		);
	Tags.Ability_Player_Skill = Manager.AddNativeGameplayTag(
		FName("Ability.Player.Skill"), 
		FString("플레이어가 스킬 사용중")
		);
	Tags.Ability_Player_Buff = Manager.AddNativeGameplayTag(
		FName("Ability.Player.Buff"),
		FString("플레이어가 버프 사용중")
		);
	//Effect_Debuff
	Tags.Effect_Debuff_Slow = Manager.AddNativeGameplayTag(
		FName("Effect.Debuff.Slow"), 
		FString("슬로우 디버프 적용중")
		);
	Tags.Effect_Debuff_Stun = Manager.AddNativeGameplayTag(
		FName("Effect.Debuff.Stun"), 
		FString("기절 디버프 적용중")
		);
	Tags.Effect_Debuff_Poison = Manager.AddNativeGameplayTag(
		FName("Effect.Debuff.Poison"),
		FString("독 디버프 적용중")
		);
	//Effect_Buff
	Tags.Effect_Buff_AttackUp = Manager.AddNativeGameplayTag(
		FName("Effect.Buff.AttackUp"), 
		FString("공격력 상승 버프 적용중")
		);
	Tags.Effect_Buff_Shield = Manager.AddNativeGameplayTag(
		FName("Effect.Buff.Shield"),
		FString("방어력 상승 버프 적용중")
		);
	//State
	Tags.State_Player_CannotCanceled = Manager.AddNativeGameplayTag(
		FName("State.Player.CannotCanceled"),
		FString("캔슬 불가 상태")
		);
	Tags.State_Player_Invincible = Manager.AddNativeGameplayTag(
		FName("State.Player.Invincible"),
		FString("무적 상태")
		);
	Tags.State_Player_Unstoppable = Manager.AddNativeGameplayTag(
		FName("State.Player.Unstoppable"),
		FString("저지 불가 상태")
		);
	//Combat
	Tags.Combat_Window_Combo = Manager.AddNativeGameplayTag(
		FName("Combat.Window.Combo"),
		FString("콤보 입력 가능 윈도우")
	);
	Tags.Combat_Window_Parry = Manager.AddNativeGameplayTag(
		FName("Combat.Window.Parry"),
		FString("패링 가능 윈도우")
	);
	//Data
	Tags.Data_Damage = Manager.AddNativeGameplayTag(
		FName("Data.Damage"),
		FString("SetByCaller용 데미지 Tag")
	);
	Tags.Data_StaminaCost = Manager.AddNativeGameplayTag(
		FName("Data.StaminaCost"),
		FString("SetByCaller용 스태미너 코스트 Tag")
	);
}

