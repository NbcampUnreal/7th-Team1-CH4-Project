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
class UEDUIManageSubsystem;
class UEDSkillSlotWidget;
class USkillComponent;
class AEDPlayerCharacter;
class UTexture2D;

UCLASS()
class ETERNALDREAMS_API UEDSkillBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// Q 키 스킬 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UEDSkillSlotWidget> QSkillSlot;

	// E 키 스킬 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UEDSkillSlotWidget> ESkillSlot;

	// Space 키 스킬 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UEDSkillSlotWidget> SpaceSkillSlot;

private:
	// 현재 소유 플레이어와 연결된 참조를 한 번 수집
	void InitializeReferences();

	// Pawn / 인벤토리 / 스킬 컴포넌트 참조가 바뀌었는지 확인 후 재바인딩
	void RefreshReferencesIfNeeded();

	// 인벤토리 슬롯 변경 델리게이트 바인딩
	void BindInventoryDelegates();
	void UnbindInventoryDelegates();

	// 스킬 쿨타임 델리게이트 바인딩
	void BindSkillDelegates();
	void UnbindSkillDelegates();

	// 슬롯 표시와 쿨타임 상태를 함께 갱신
	void RefreshSkillBar();

	// 현재 장착된 아이템 기준으로 Q/E/Space 슬롯 아이콘을 갱신
	void RefreshSkillSlotDisplay() const;

	// 현재 활성 쿨타임 태그를 조회해 슬롯 오버레이를 갱신
	void RefreshCooldownState() const;

	// PrimaryAssetId로 스킬/무기 아이템 데이터를 조회
	const UEDInventoryItemDataAsset* ResolveItemData(const FPrimaryAssetId& ItemId) const;

	// 토스트 표시용 아이템 이름을 해석
	FText ResolveItemDisplayName(const FPrimaryAssetId& ItemId) const;

	// 쿨타임 태그로 현재 남은 시간과 최대 시간을 조회
	bool ResolveCooldownFromTag(const FGameplayTag& CooldownTag, float& OutRemainingTime, float& OutMaxCooldownTime) const;

	// Tick에서 안전하게 한 번 더 갱신하도록 플래그 설정
	void RequestDeferredRefresh();

	// 인벤토리 슬롯에서 드래그한 아이템을 대상 스킬 슬롯으로 장착 시도
	void HandleSkillSlotDropped(EEDSkillSlotType TargetSkillSlotType, int32 SourceSlotIndex);
	void HandleFirstSkillSlotDropped(int32 SourceSlotIndex);
	void HandleSecondSkillSlotDropped(int32 SourceSlotIndex);

	// 스킬 슬롯 더블 클릭 시 인벤토리로 해제 시도
	void HandleSkillSlotDoubleClicked(EEDSkillSlotType SkillSlotType);
	void HandleFirstSkillSlotDoubleClicked();
	void HandleSecondSkillSlotDoubleClicked();

	// 인벤토리 액션 성공/실패 토스트 출력
	void ShowInventoryFailure(EEDInventoryActionFailure Failure) const;
	void ShowInventorySuccess(const FText& TargetName, const FText& ActionName) const;

	// 스킬 해제 전에 플레이어 인벤토리에 빈칸이 있는지 확인
	bool HasEmptyInventorySlot() const;

	// 현재 장착된 스킬 아이템 이름을 슬롯 타입 기준으로 조회
	FText ResolveSkillItemDisplayName(EEDSkillSlotType SkillSlotType) const;

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
	// 현재 HUD를 소유한 플레이어 캐릭터
	UPROPERTY(Transient)
	TObjectPtr<AEDPlayerCharacter> CachedPlayerCharacter;

	// 장착/해제 요청에 사용하는 인벤토리 컴포넌트
	UPROPERTY(Transient)
	TObjectPtr<UEDInventoryComponent> CachedInventoryComponent;

	// 현재 스킬 태그와 쿨타임 델리게이트를 제공하는 스킬 컴포넌트
	UPROPERTY(Transient)
	TObjectPtr<USkillComponent> CachedSkillComponent;

	// 활성 쿨타임 태그를 조회하기 위한 ASC
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	// 참조 재획득 직후 다음 Tick에서 전체 갱신을 한 번 더 수행
	bool bDeferredRefreshRequested = false;
};
