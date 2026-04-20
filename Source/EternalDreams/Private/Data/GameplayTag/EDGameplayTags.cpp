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
	Tags.Ability_Monster_Blink = Manager.AddNativeGameplayTag(
		FName("Ability.Monster.Blink"),
		FString("보스 점멸 어빌리티")
	);
	//Player_BasicAttack
	Tags.Player_BasicAttack_Hammer = Manager.AddNativeGameplayTag(
		FName("Player.BasicAttack.Hammer"),
		FString("해머 기본공격")
	);
	Tags.Player_BasicAttack_Sword = Manager.AddNativeGameplayTag(
		FName("Player.BasicAttack.Sword"),
		FString("검 기본공격")
	);

	Tags.Player_BasicAttack_Staff = Manager.AddNativeGameplayTag(
		FName("Player.BasicAttack.Staff"),
		FString("스태프 기본공격")
	);

	Tags.Player_BasicAttack_Bow = Manager.AddNativeGameplayTag(
		FName("Player.BasicAttack.Bow"),
		FString("활 기본공격")
	);
	
	//Player_Evade
	Tags.Player_Evade_Hammer = Manager.AddNativeGameplayTag(
		FName("Player.Evade.Hammer"),
		FString("해머 회피")
	);

	Tags.Player_Evade_Sword = Manager.AddNativeGameplayTag(
		FName("Player.Evade.Sword"),
		FString("검 회피")
	);

	Tags.Player_Evade_Staff = Manager.AddNativeGameplayTag(
		FName("Player.Evade.Staff"),
		FString("스태프 회피")
	);

	Tags.Player_Evade_Bow = Manager.AddNativeGameplayTag(
		FName("Player.Evade.Bow"),
		FString("활 회피")
	);
	
	//Player_Skill
	Tags.Player_Skill_Whirlwind = Manager.AddNativeGameplayTag(
		FName("Player.Skill.Whirlwind"),
		FString("스킬: 휠윈드")
	);
	Tags.Player_Skill_Whirllaser = Manager.AddNativeGameplayTag(
	FName("Player.Skill.Whirllaser"),
	FString("스킬: 휠레이저")
);
	Tags.Player_Skill_BasicKnockBack = Manager.AddNativeGameplayTag(
	FName("Player.Skill.BasicKnockBack"),
	FString("스킬: 베이직넉백")
);
	
	//CoolDown
	Tags.CoolDown_Evade_Hammer = Manager.AddNativeGameplayTag(
		FName("CoolDown.Evade.Hammer"),
	FString("쿨다운: 해머 회피")
	);

	Tags.CoolDown_Evade_Sword = Manager.AddNativeGameplayTag(
		FName("CoolDown.Evade.Sword"),
		FString("쿨다운: 소드 회피")
	);

	Tags.CoolDown_Evade_Staff = Manager.AddNativeGameplayTag(
		FName("CoolDown.Evade.Staff"),
		FString("쿨다운: 스태프 회피")
	);

	Tags.CoolDown_Evade_Bow = Manager.AddNativeGameplayTag(
		FName("CoolDown.Evade.Bow"),
		FString("쿨다운: 활 회피")
	);

	Tags.CoolDown_Skill_Whirlwind = Manager.AddNativeGameplayTag(
		FName("CoolDown.Skill.Whirlwind"),
		FString("쿨다운: 휠윈드")
	);
	
	Tags.CoolDown_Skill_Whirllaser = Manager.AddNativeGameplayTag(
	FName("CoolDown.Skill.Whirllaser"),
	FString("쿨다운: 휠레이저")
);
	
	Tags.CoolDown_Skill_BasicKnockBack = Manager.AddNativeGameplayTag(
	FName("CoolDown.Skill.BasicKnockBack"),
	FString("쿨다운: 베이직넉백")
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
	Tags.State_Player_Stop = Manager.AddNativeGameplayTag(
	FName("State.Player.Stop"),
	FString("이동 불가 상태")
);

	Tags.State_Player_RestrictedArea = Manager.AddNativeGameplayTag(
		FName("State.Player.RestrictedArea"),
		FString("금지구역 진입 상태")
	);
	Tags.State_Debuff_RestrictedArea = Manager.AddNativeGameplayTag(
		FName("State.Debuff.RestrictedArea"),
		FString("금지구역 상태")
		);
	
	Tags.State_Dead = Manager.AddNativeGameplayTag(
		FName("State.Dead"),
		FString("사망 상태")
		);
	
	
	//Phase
	Tags.Phase_Day1_Day = Manager.AddNativeGameplayTag(
		FName("Phase.Day1.Day"),
		FString("1일차 낮")
	);
	Tags.Phase_Day1_Night = Manager.AddNativeGameplayTag(
		FName("Phase.Day1.Night"),
		FString("1일차 밤")
	);
	Tags.Phase_Day2_Day = Manager.AddNativeGameplayTag(
		FName("Phase.Day2.Day"),
		FString("2일차 낮")
	);
	Tags.Phase_Day2_Night = Manager.AddNativeGameplayTag(
		FName("Phase.Day2.Night"),
		FString("2일차 밤")
	);
	Tags.Phase_Day3_Day = Manager.AddNativeGameplayTag(
		FName("Phase.Day3.Day"),
		FString("3일차 낮")
	);
	Tags.Phase_Day3_Night = Manager.AddNativeGameplayTag(
		FName("Phase.Day3.Night"),
		FString("3일차 밤")
	);
	Tags.Phase_Day4_Day = Manager.AddNativeGameplayTag(
		FName("Phase.Day4.Day"),
		FString("4일차 낮")
	);
	Tags.Phase_Day4_Night = Manager.AddNativeGameplayTag(
		FName("Phase.Day4.Night"),
		FString("4일차 밤")
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
	Tags.Data_StatAdd = Manager.AddNativeGameplayTag(
		FName("Data.StatAdd"),
FString("SetByCaller용 스탯 + 증가 Tag")
	);
	Tags.Data_StatAdd_Strength = Manager.AddNativeGameplayTag(
	FName("Data.StatAdd.Strength"),
FString("SetByCaller용 Strength + 증가 Tag")
);
	Tags.Data_StatAdd_Dexterity = Manager.AddNativeGameplayTag(
	FName("Data.StatAdd.Dexterity"),
FString("SetByCaller용 Dexterity + 증가 Tag")
);
	Tags.Data_StatAdd_Intelligence = Manager.AddNativeGameplayTag(
	FName("Data.StatAdd.Intelligence"),
FString("SetByCaller용 Intelligence + 증가 Tag")
);
	
	
	Tags.Data_StatMul = Manager.AddNativeGameplayTag(
		FName("Data.StatMul"),
		FString("SetByCaller용 스탯 % 증가 Tag")
	);
	Tags.Data_CoolTime = Manager.AddNativeGameplayTag(
	FName("Data.CoolTime"),
	FString("SetByCaller용 CoolTime 증가 Tag")
	);
	
	
	
	//Inventory-Item
	Tags.Item_Weapon_Sword = Manager.AddNativeGameplayTag(
		FName("Item.Weapon.Sword"),
		FString("무기 - 검")
	);
	Tags.Item_Weapon_Hammer = Manager.AddNativeGameplayTag(
		FName("Item.Weapon.Hammer"),
		FString("무기 - 망치")
	);
	Tags.Item_Weapon_Bow = Manager.AddNativeGameplayTag(
		FName("Item.Weapon.Bow"),
		FString("무기 - 활")
	);
	Tags.Item_Weapon_Staff = Manager.AddNativeGameplayTag(
		FName("Item.Weapon.Staff"),
		FString("무기 - 지팡이")
	);
	
	Tags.Item_Armor_TopArmor = Manager.AddNativeGameplayTag(
		FName("Item.Armor.TopArmor"),
		FString("방어구 - 상의")
	);
	Tags.Item_Armor_BottomArmor = Manager.AddNativeGameplayTag(
		FName("Item.Armor.BottomArmor"),
		FString("방어구 - 하의")
	);
	
	Tags.Item_Consumable = Manager.AddNativeGameplayTag(
		FName("Item.Consumable"),
		FString("소비 아이템")
	);
	Tags.Item_Ingredient_Base = Manager.AddNativeGameplayTag(
		FName("Item.Ingredient.Base"),
		FString("재료 아이템 - 기본")
	);
	Tags.Item_Ingredient_Special = Manager.AddNativeGameplayTag(
		FName("Item.Ingredient.Special"),
		FString("재료 아이템 - 특수")
	);

}
