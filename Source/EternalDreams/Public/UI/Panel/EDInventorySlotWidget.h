#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventorySlotWidget.generated.h"

class UBorder;
class UDragDropOperation;
class UEDInventoryComponent;
class UImage;
class UTextBlock;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotClicked, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotRightClicked, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventorySlotDoubleClicked, int32);

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
	void SetItemState(const FText& InItemName, int32 InQuantity, EEDItemRarity InRarity, UTexture2D* InIconTexture = nullptr);

	// 슬롯 인덱스 설정
	void SetSlotIndex(int32 InSlotIndex);

	// 선택 상태 강조 표시
	void SetSelectedState(bool bSelected);

	// 드래그 시작 시 원본 인벤토리 식별용 컴포넌트 설정
	void SetSourceInventoryComponent(UEDInventoryComponent* InSourceInventoryComponent);

	// 슬롯 드래그 허용 여부 설정
	void SetSupportsItemDrag(bool bInSupportsItemDrag);

	UEDInventoryComponent* GetSourceInventoryComponent() const { return SourceInventoryComponent; }

	FOnEDInventorySlotClicked OnSlotClicked;
	FOnEDInventorySlotRightClicked OnSlotRightClicked;
	FOnEDInventorySlotDoubleClicked OnSlotDoubleClicked;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

protected:
	// 빈 슬롯 안내 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> EmptyText;

	// 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> ItemNameText;

	// 수량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> QuantityText;

	// 아이템 아이콘 이미지
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UImage> ItemIconImage;

	// 희귀도 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBorder> RarityAccent;

	// 선택 상태 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UBorder> SelectionBorder;

private:
	// 현재 슬롯 인덱스
	int32 SlotIndex = INDEX_NONE;

	// 현재 선택 상태
	bool bIsSelected = false;

	// 현재 슬롯에 실제 아이템이 있는지
	bool bHasItem = false;

	// 현재 아이템 수량
	int32 CurrentQuantity = 0;
	FText CurrentItemName;
	EEDItemRarity CurrentRarity = EEDItemRarity::Normal;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> CurrentIconTexture = nullptr;

	// 현재 슬롯에서 드래그 시작을 허용할지 여부
	bool bSupportsItemDrag = true;

	// 드래그 시작 시 원본 인벤토리 판별용 참조
	UPROPERTY(Transient)
	TObjectPtr<UEDInventoryComponent> SourceInventoryComponent;
};
