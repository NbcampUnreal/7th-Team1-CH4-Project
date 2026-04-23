#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDEquipmentSlotWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDEquipmentSlotDoubleClicked, EEDEquippableType);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDEquipmentSlotClicked, EEDEquippableType);

UCLASS()
class ETERNALDREAMS_API UEDEquipmentSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 빈 장비 슬롯 상태로 표시
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEmptyState(const FText& InSlotTypeName);

	// 장비 아이템이 들어 있는 슬롯 상태로 표시
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetItemState(const FText& InSlotTypeName, const FText& InItemName, EEDItemRarity InRarity, UTexture2D* InIconTexture = nullptr);

	// 장비 슬롯 타입 설정
	void SetSlotType(EEDEquippableType InSlotType);

	// 선택 상태 강조 표시
	void SetSelectedState(bool bSelected);

	FOnEDEquipmentSlotClicked OnEquipmentSlotClicked;
	FOnEDEquipmentSlotDoubleClicked OnEquipmentSlotDoubleClicked;

protected:
	// 슬롯 타입명 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTextBlock> SlotTypeText;

	// 장착 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTextBlock> ItemNameText;

	// 빈 슬롯 안내 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTextBlock> EmptyText;

	// 장착 아이템 아이콘 이미지
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UImage> ItemIconImage;

	// 희귀도 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBorder> RarityAccent;

	// 선택 상태 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBorder> SelectionBorder;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	// 현재 슬롯 타입
	EEDEquippableType SlotType = EEDEquippableType::None;

	// 현재 선택 상태
	bool bIsSelected = false;
};
