#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventorySlotWidget.generated.h"

class UBorder;
class UTextBlock;

// 슬롯 좌클릭 이벤트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotClicked, int32);

// 슬롯 우클릭 이벤트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotRightClicked, int32);

// 슬롯 더블 클릭 이벤트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotDoubleClicked, int32);

/**
 * 인벤토리 슬롯 한 칸을 표시하는 공용 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDInventorySlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯을 빈 상태로 표시한다.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetEmptyState();

	// 슬롯에 아이템 이름, 수량, 희귀도 정보를 표시
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemState(const FText& InItemName, int32 InQuantity, EEDItemRarity InRarity);

	// 이 위젯이 가리키는 슬롯 인덱스를 설정
	void SetSlotIndex(int32 InSlotIndex);

	// 슬롯 선택 상태를 시각적으로 갱신
	void SetSelectedState(bool bSelected);

	// 좌클릭 이벤트
	FOnEDInventorySlotClicked OnSlotClicked;

	// 우클릭 이벤트
	FOnEDInventorySlotRightClicked OnSlotRightClicked;

	// 더블 클릭 이벤트
	FOnEDInventorySlotDoubleClicked OnSlotDoubleClicked;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	// 빈 슬롯일 때 표시되는 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> EmptyText;

	// 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> ItemNameText;

	// 아이템 수량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> QuantityText;

	// 희귀도 강조색 영역
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBorder> RarityAccent;

	// 선택 상태 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBorder> SelectionBorder;

private:
	// 현재 위젯이 가리키는 슬롯 인덱스
	int32 SlotIndex = INDEX_NONE;

	// 현재 선택 상태 여부
	bool bIsSelected = false;
};
