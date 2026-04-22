#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDSkillSlotWidget.generated.h"

class UBorder;
class UDragDropOperation;
class UImage;
class USizeBox;
class UTextBlock;
class UTexture2D;

USTRUCT()
struct FEDSkillSlotDisplayData
{
	GENERATED_BODY()

	// 슬롯에 표시할 아이콘 텍스처
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	// 슬롯 하단에 표시할 입력 키 라벨
	FText KeyLabel;

	// 현재 슬롯에 연결된 스킬 태그
	FGameplayTag SkillTag = FGameplayTag::EmptyTag;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDSkillSlotDropped, int32);
DECLARE_MULTICAST_DELEGATE(FOnEDSkillSlotDoubleClicked);

UCLASS()
class ETERNALDREAMS_API UEDSkillSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 아이콘, 키 라벨, 스킬 태그를 받아 슬롯 표시를 갱신
	void SetSlotDisplayData(const FEDSkillSlotDisplayData& InDisplayData);

	// 쿨타임 시작 시점의 남은 시간/최대 시간을 받아 오버레이를 활성화
	void StartCooldown(float InRemainingTime, float InMaxCooldownTime);

	// 쿨타임 오버레이와 숫자 표시를 초기화
	void ClearCooldown();

	// 드롭 시 어떤 스킬 슬롯으로 장착할지 타겟 타입을 지정
	void SetTargetSkillSlotType(EEDSkillSlotType InTargetSkillSlotType);

	// 인벤토리 슬롯에서 드래그한 아이템을 이 슬롯에 드롭했을 때 브로드캐스트
	FOnEDSkillSlotDropped OnSkillSlotDropped;

	// 스킬 슬롯 더블 클릭 시 브로드캐스트
	FOnEDSkillSlotDoubleClicked OnSkillSlotDoubleClicked;

protected:
	// 스킬 아이콘 이미지
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UImage> SkillIconImage;

	// 입력 키 라벨
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UTextBlock> KeyText;

	// 쿨타임 오버레이 전체를 감싸는 SizeBox
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<USizeBox> CooldownMaskSizeBox;

	// 쿨타임 동안 슬롯 전체를 덮는 오버레이
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UBorder> CooldownOverlayBorder;

	// 남은 쿨타임 숫자 표시
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Skill")
	TObjectPtr<UTextBlock> CooldownValueText;

	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// 현재 슬롯에 표시 중인 아이콘
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> CachedIconTexture;

	// 현재 슬롯에 표시 중인 키 라벨
	FText CachedKeyLabel;

	// 현재 슬롯에 연결된 스킬 태그
	FGameplayTag CachedSkillTag = FGameplayTag::EmptyTag;

	// 드롭 시 장착 대상이 되는 스킬 슬롯 타입
	EEDSkillSlotType TargetSkillSlotType = EEDSkillSlotType::FirstSkill;

	// 쿨타임 애니메이션 상태
	bool bIsOnCooldown = false;
	float CooldownStartWorldTime = 0.0f;
	float CooldownEndWorldTime = 0.0f;
	float CooldownDuration = 0.0f;
};
