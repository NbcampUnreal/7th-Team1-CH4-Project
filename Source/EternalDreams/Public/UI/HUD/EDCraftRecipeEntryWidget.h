#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDCraftRecipeEntryWidget.generated.h"

class UBorder;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDCraftRecipeEntryClicked, FName);

/**
 * 제작 레시피 리스트의 한 줄을 담당하는 위젯
 * 결과 아이템 이름, 제작 가능 여부, 선택 상태를 표시
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftRecipeEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 레시피 데이터를 반영해 한 줄 UI를 갱신
	void SetRecipeViewData(const FEDCraftRecipeViewData& InRecipeData);

	// 선택 여부에 따라 시각 상태를 갱신
	void SetSelectedState(bool bSelected);

	// 이 엔트리가 어떤 레시피 Row를 나타내는지 반환
	FName GetRecipeRowId() const { return RecipeRowId; }

	// 레시피 엔트리 클릭 이벤트
	FOnEDCraftRecipeEntryClicked OnRecipeEntryClicked;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	// 레시피 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> RecipeNameText;

	// 제작 가능 여부 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> CraftStateText;

	// 희귀도 강조 라인
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> RarityAccent;

	// 선택 강조 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> SelectionBorder;

private:
	FLinearColor GetRarityColor(EEDItemRarity InRarity) const;

	// 현재 레시피 Row ID
	FName RecipeRowId = NAME_None;
};
