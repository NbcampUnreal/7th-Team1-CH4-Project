#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Item/Core/EDItemTypes.h"
#include "EDEquipmentSlotWidget.generated.h"

class UBorder;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDEquipmentSlotDoubleClicked, EEDEquippableType);

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
	
	// 마우스 더블 클릭 입력 처리
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	// 희귀도에 맞는 색상을 반환
	FLinearColor GetRarityColor(EEDItemRarity InRarity) const;
	
	// 현재 장비 슬롯 타입
	EEDEquippableType SlotType = EEDEquippableType::None;
};
