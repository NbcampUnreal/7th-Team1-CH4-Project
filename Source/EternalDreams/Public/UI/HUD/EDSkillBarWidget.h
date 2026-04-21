#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDSkillBarWidget.generated.h"

class UAbilitySystemComponent;
class UEDInventoryComponent;
class UEDInventoryItemDataAsset;
class UEDSkillSlotWidget;
class USkillComponent;
class AEDPlayerCharacter;
class UTexture2D;

/**
 * 플레이어 하단 HUD에 표시될 3칸 스킬 바 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDSkillBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// Q 스킬 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UEDSkillSlotWidget> QSkillSlot;

	// E 스킬 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UEDSkillSlotWidget> ESkillSlot;

	// Space 스킬 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UEDSkillSlotWidget> SpaceSkillSlot;

private:
	// 현재 로컬 플레이어를 기준으로 캐릭터 / 인벤토리 / 스킬 컴포넌트를 찾음
	void InitializeReferences();

	// Pawn / Inventory / SkillComponent / ASC 참조가 바뀌면 다시 바인딩하고 표시를 갱신
	void RefreshReferencesIfNeeded();

	// 인벤토리 장착 슬롯 변화 이벤트를 구독
	void BindInventoryDelegates();

	// 기존 인벤토리 이벤트 구독을 해제
	void UnbindInventoryDelegates();

	// 스킬 쿨타임 델리게이트를 구독
	void BindSkillDelegates();

	// 기존 스킬 쿨타임 델리게이트 구독을 해제
	void UnbindSkillDelegates();

	// 3칸 전체 슬롯 정보를 현재 장착 상태 기준으로 다시 그림
	void RefreshSkillBar();

	// 현재 장착 중인 첫 번째 스킬 / 두 번째 스킬 / 무기 아이템 아이콘을 다시 읽음
	void RefreshSkillSlotDisplay() const;

	// 현재 활성화 중인 쿨타임 태그를 기준으로 각 슬롯 쿨타임 상태를 즉시 동기화
	void RefreshCooldownState() const;

	// 특정 아이템 에셋 ID로부터 아이콘 데이터를 찾음
	const UEDInventoryItemDataAsset* ResolveItemData(const FPrimaryAssetId& ItemId) const;

	// 특정 쿨타임 태그의 남은 시간과 최대 시간을 ASC에서 조회
	bool ResolveCooldownFromTag(const FGameplayTag& CooldownTag, float& OutRemainingTime, float& OutMaxCooldownTime) const;

	// 인벤토리 장착 상태 변경을 받은 뒤 다음 Tick에서 표시를 다시 그리도록 요청
	void RequestDeferredRefresh();

	UFUNCTION()
	void HandleWeaponSlotChanged(const FGameplayTagContainer& MainItemTags, const FGameplayTagContainer& SpecialItemTags, const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags);

	UFUNCTION()
	void HandleFirstSkillSlotChanged(const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags);

	UFUNCTION()
	void HandleSecondSkillSlotChanged(const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags);

	UFUNCTION()
	void HandleQSkillCoolTime(float SkillCoolTime, float MaxSkillCoolTime);

	UFUNCTION()
	void HandleESkillCoolTime(float SkillCoolTime, float MaxSkillCoolTime);

	UFUNCTION()
	void HandleSpaceSkillCoolTime(float SkillCoolTime, float MaxSkillCoolTime);

private:
	UPROPERTY(Transient)
	TObjectPtr<AEDPlayerCharacter> CachedPlayerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UEDInventoryComponent> CachedInventoryComponent;

	UPROPERTY(Transient)
	TObjectPtr<USkillComponent> CachedSkillComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	bool bDeferredRefreshRequested = false;
};
