#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDCraftIngredientEntryWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;

/**
 * 선택된 제작 레시피의 재료 한 줄을 표시하는 위젯
 * 아이콘, 이름, 필요/보유 수량, 충족 여부를 표시
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftIngredientEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 재료 뷰 데이터를 반영해 한 줄 UI 갱신
	void SetIngredientViewData(const FEDCraftIngredientViewData& InIngredientData);

protected:
	// 재료 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> IngredientIconImage;

	// 재료 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> IngredientNameText;

	// 필요/보유 수량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> QuantityText;

	// 충족 여부 강조용 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> SatisfiedAccent;
};
