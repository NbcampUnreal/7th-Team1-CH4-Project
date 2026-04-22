#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CommonUserWidget.h"
#include "EDSkillSlotWidget.generated.h"

class UBorder;
class UImage;
class USizeBox;
class UTextBlock;
class UTexture2D;

USTRUCT()
struct FEDSkillSlotDisplayData
{
	GENERATED_BODY()

	TObjectPtr<UTexture2D> IconTexture = nullptr;
	FText KeyLabel;
	FGameplayTag SkillTag = FGameplayTag::EmptyTag;
};

/**
 * 하단 스킬 바에서 개별 스킬 하나를 표시하는 슬롯 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDSkillSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 슬롯 표시 데이터(아이콘, 키 라벨, 스킬 태그)를 갱신
	void SetSlotDisplayData(const FEDSkillSlotDisplayData& InDisplayData);

	// 쿨타임 시작 시 남은 시간과 최대 시간을 기준으로 마스크를 활성화
	void StartCooldown(float InRemainingTime, float InMaxCooldownTime);

	// 쿨타임이 끝났을 때 마스크를 제거
	void ClearCooldown();

protected:
	// 스킬 아이콘 이미지
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UImage> SkillIconImage;

	// 슬롯 아래에 표시될 키 라벨
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UTextBlock> KeyText;

	// 쿨타임 마스크 높이를 제어할 SizeBox
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<USizeBox> CooldownMaskSizeBox;

	// 위에서 아래로 사라지는 검은 오버레이
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UBorder> CooldownOverlayBorder;

	// 남은 쿨타임 수치를 표시할 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UTextBlock> CooldownValueText;

private:
	void RefreshCooldownOverlay(float SlotHeight) const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> CachedIconTexture;

	FText CachedKeyLabel;
	FGameplayTag CachedSkillTag = FGameplayTag::EmptyTag;

	bool bIsOnCooldown = false;
	float CooldownStartWorldTime = 0.0f;
	float CooldownEndWorldTime = 0.0f;
	float CooldownDuration = 0.0f;
};
