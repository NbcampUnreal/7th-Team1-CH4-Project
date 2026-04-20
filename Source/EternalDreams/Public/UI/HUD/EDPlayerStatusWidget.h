#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayEffectTypes.h"
#include "EDPlayerStatusWidget.generated.h"

class UAbilitySystemComponent;
class UEDBaseAttributeSet;
class UEDPlayerAttributeSet;
class AEDPlayerCharacter;
class AEDPlayerState;
class UImage;
class UProgressBar;
class UTextBlock;
struct FOnAttributeChangeData;

UCLASS()
class ETERNALDREAMS_API UEDPlayerStatusWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 플레이어 이름 표시용 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> PlayerNameText;

	// 플레이어 이미지 슬롯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UImage> PlayerImage;

	// 체력 바
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UProgressBar> HPBar;

	// 체력 수치 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> HPValueText;

	// 방어력 수치 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> DefensiveValueText;

	// 이동속도 수치 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> MoveSpeedValueText;

	// 힘 스탯 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> StrengthValueText;

	// 민첩 스탯 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> DexterityValueText;

	// 지능 스탯 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> IntelligenceValueText;

	// 공격 속도 텍스트 (추후 추가 예정)
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PlayerStatus")
	TObjectPtr<UTextBlock> AttackSpeedValueText;

private:
	// 현재 HUD가 참조 중인 로컬 플레이어 캐릭터
	UPROPERTY(Transient)
	TObjectPtr<AEDPlayerCharacter> CachedPlayerCharacter;

	// 현재 HUD가 참조 중인 로컬 플레이어 스테이트
	UPROPERTY(Transient)
	TObjectPtr<AEDPlayerState> CachedPlayerState;

	// 현재 HUD가 구독 중인 ASC
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	// 체력/방어력/이동속도 AttributeSet
	UPROPERTY(Transient)
	TObjectPtr<UEDBaseAttributeSet> CachedBaseAttributeSet;

	// 힘/민첩/지능 AttributeSet
	UPROPERTY(Transient)
	TObjectPtr<UEDPlayerAttributeSet> CachedPlayerAttributeSet;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle DefensiveChangedHandle;
	FDelegateHandle MaxDefensiveChangedHandle;
	FDelegateHandle WalkSpeedChangedHandle;
	FDelegateHandle StrengthChangedHandle;
	FDelegateHandle DexterityChangedHandle;
	FDelegateHandle IntelligenceChangedHandle;

	// 현재 로컬 플레이어 기준으로 캐릭터/스테이트/ASC를 찾음
	void InitializePlayerReferences();

	// ASC Attribute 변화 델리게이트를 구독
	void BindAttributeDelegates();

	// 기존에 구독한 Attribute 변화 델리게이트를 해제
	void UnbindAttributeDelegates();

	// 현재 참조 중인 데이터로 위젯 전체를 한 번 갱신
	void RefreshAllDisplay() const;

	// 플레이어 이름 표시를 갱신
	void RefreshPlayerName() const;

	// 초상 이미지 표시를 갱신
	void RefreshPortrait() const;

	// 체력 바와 수치를 갱신
	void RefreshHealthDisplay() const;

	// 방어력 수치를 갱신
	void RefreshDefensiveDisplay() const;

	// 이동속도 표시를 갱신
	void RefreshMoveSpeedDisplay() const;

	// 힘/민첩/지능 표시를 갱신
	void RefreshStatDisplay() const;

	// 공격속도 표시를 갱신
	void RefreshAttackSpeedDisplay() const;

	void HandleHealthChanged(const FOnAttributeChangeData& Data) const;
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data) const;
	void HandleDefensiveChanged(const FOnAttributeChangeData& Data) const;
	void HandleMaxDefensiveChanged(const FOnAttributeChangeData& Data) const;
	void HandleWalkSpeedChanged(const FOnAttributeChangeData& Data) const;
	void HandleStrengthChanged(const FOnAttributeChangeData& Data) const;
	void HandleDexterityChanged(const FOnAttributeChangeData& Data) const;
	void HandleIntelligenceChanged(const FOnAttributeChangeData& Data) const;
};
