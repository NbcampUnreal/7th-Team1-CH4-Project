#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDEquipmentSlotWidget.generated.h"

class UBorder;
class UTextBlock;

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

	// 장비가 장착된 상태로 표시
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetItemState(const FText& InSlotTypeName, const FText& InItemName, EEDItemRarity InRarity);

	// 장비 슬롯 타입 설정
	void SetSlotType(EEDEquippableType InSlotType);
	
	// 선택 여부에 따른 시각 상태 갱신
	void SetSelectedState(bool bSelected);
	
	// 장비 슬롯 클릭 이벤트
	FOnEDEquipmentSlotClicked OnEquipmentSlotClicked;
	
	// 장비 슬롯 더블 클릭 이벤트
	FOnEDEquipmentSlotDoubleClicked OnEquipmentSlotDoubleClicked;

protected:
	// 슬롯 종류 표시 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTextBlock> SlotTypeText;

	// 장착된 아이템 이름 표시 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTextBlock> ItemNameText;

	// 빈 슬롯 상태 표시 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UTextBlock> EmptyText;

	// 희귀도 강조 라인
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBorder> RarityAccent;

	// 장비 슬롯 선택 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UBorder> SelectionBorder;

	// 마우스 클릭 입력 처리
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 마우스 더블 클릭 입력 처리
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
private:
	// 현재 장비 슬롯 타입
	EEDEquippableType SlotType = EEDEquippableType::None;
	
	// 현재 선택 상태
	bool bIsSelected = false;
};
