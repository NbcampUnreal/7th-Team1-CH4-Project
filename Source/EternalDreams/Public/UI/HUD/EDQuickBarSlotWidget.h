#pragma once

#include "CoreMinimal.h"
#include "UI/Panel/EDInventorySlotWidget.h"
#include "EDQuickBarSlotWidget.generated.h"

class UEDInventoryComponent;
class UTexture2D;

// 퀵바 슬롯 좌클릭 이벤트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDQuickBarSlotClicked, int32);

// 퀵바 슬롯 더블 클릭 이벤트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDQuickBarSlotDoubleClicked, int32);

// 퀵바 슬롯 드래그/드롭 이벤트
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEDQuickBarSlotDroppedOnSlot, UEDInventoryComponent*, int32, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDQuickBarSlotDroppedOutside, int32);

UCLASS()
class ETERNALDREAMS_API UEDQuickBarSlotWidget : public UEDInventorySlotWidget
{
	GENERATED_BODY()

public:
	// 슬롯 인덱스 설정
	void SetSlotIndex(int32 InSlotIndex);

	// 좌클릭 시 호출되는 델리게이트
	FOnEDQuickBarSlotClicked OnQuickBarSlotClicked;

	// 더블 클릭 시 호출되는 델리게이트
	FOnEDQuickBarSlotDoubleClicked OnQuickBarSlotDoubleClicked;
	
	// 다른 슬롯 위에 드롭됐을 때 호출되는 델리게이트
	FOnEDQuickBarSlotDroppedOnSlot OnQuickBarSlotDroppedOnSlot;

	// 퀵바 밖으로 드롭됐을 때 호출되는 델리게이트
	FOnEDQuickBarSlotDroppedOutside OnQuickBarSlotDroppedOutside;
	
	// 빈 슬롯 상태로 표시
	void SetEmptyState();
	
	// 아이템이 들어 있는 슬롯 상태로 표시
	void SetItemState(const FText& InItemName, int32 InQuantity, EEDItemRarity InRarity, UTexture2D* InIconTexture = nullptr);

protected:
	// 좌클릭 입력 처리
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// 더블 클릭 입력 처리
	virtual FReply
	NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	// 현재 슬롯 인덱스
	int32 SlotIndex = INDEX_NONE;
	
	// 현재 슬롯에 실제 아이템이 있는지
	bool bHasItem = false;

	// 현재 슬롯 수량
	int32 CurrentQuantity = 0;

	// 현재 아이템 이름
	FText QuickBarCurrentItemName;

	// 현재 아이템 희귀도
	EEDItemRarity QuickBarCurrentRarity = EEDItemRarity::Normal;

	// 현재 아이템 아이콘
	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> QuickBarCurrentIconTexture = nullptr;
	
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
