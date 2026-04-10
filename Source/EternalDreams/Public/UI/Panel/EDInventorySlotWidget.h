#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventorySlotWidget.generated.h"

class UBorder;
class UTextBlock;
class UEDEquipmentSlotWidget;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotClicked, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotRightClicked, int32);

UCLASS()
class ETERNALDREAMS_API UEDInventorySlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 빈 슬롯 상태로 표시
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetEmptyState();

	// 아이템이 들어 있는 슬롯 상태로 표시
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemState(const FText& InItemName, int32 InQuantity, EEDItemRarity InRarity);
	
	// 슬롯 인덱스 설정
	void SetSlotIndex(int32 InSlotIndex);

	// 선택 여부에 따라 시각 상태를 갱신
	void SetSelectedState(bool bSelected);

	// 좌클릭 이벤트
	FOnEDInventorySlotClicked OnSlotClicked;

	// 우클릭 이벤트
	FOnEDInventorySlotRightClicked OnSlotRightClicked;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	// 빈 슬롯 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> EmptyText;

	// 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> ItemNameText;

	// 수량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> QuantityText;

	// 희귀도 강조 라인
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBorder> RarityAccent;
	
	// 슬롯 선택 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBorder> SelectionBorder;

private:
	// 희귀도에 맞는 색상을 반환
	FLinearColor GetRarityColor(EEDItemRarity InRarity) const;
	
	// 현재 슬롯 인덱스
	int32 SlotIndex = INDEX_NONE;

	// 현재 선택 상태
	bool bIsSelected = false;
};
