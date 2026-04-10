#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventorySlotWidget.generated.h"

class UBorder;
class UTextBlock;
class UEDEquipmentSlotWidget;

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

private:
	// 희귀도에 맞는 색상을 반환
	FLinearColor GetRarityColor(EEDItemRarity InRarity) const;
};
