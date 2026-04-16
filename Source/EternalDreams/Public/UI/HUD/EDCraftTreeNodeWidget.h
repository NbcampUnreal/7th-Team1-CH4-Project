#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDCraftTreeNodeWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;

/**
 * 제작 성장 트리에서 아이템 노드 하나를 표시하는 위젯
 * 아이콘, 이름, 수량, 보유 여부를 한 번에 보여주는 용도
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftTreeNodeWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 노드 뷰 데이터를 받아 트리 노드 표시를 갱신
	void SetTreeNodeViewData(const FEDCraftTreeNodeViewData& InNodeData);

protected:
	// 아이템 아이콘 이미지
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> ItemIconImage;

	// 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> ItemNameText;

	// 보유 수량 / 필요 수량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> QuantityText;

	// 희귀도 강조 라인 또는 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> RarityAccent;

	// 보유 여부 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> SatisfiedBorder;

private:
	FLinearColor GetRarityColor(EEDItemRarity InRarity) const;
};
