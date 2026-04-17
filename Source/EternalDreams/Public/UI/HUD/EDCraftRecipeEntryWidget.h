#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDCraftRecipeEntryWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDCraftRecipeEntryClicked, FName);

// 레시피 카드 위젯이 화면에 그리기 위해 사용하는 표시 데이터
struct FEDCraftRecipeEntryDisplayData
{
	FName RowId = NAME_None;
	FText ResultItemName;
	EEDItemRarity ResultRarity = EEDItemRarity::Normal;
	TObjectPtr<UTexture2D> ResultIconTexture = nullptr;
	bool bCanCraft = false;
};

/**
 * 제작 가능한 아이템 목록에서 레시피 카드 한 칸을 표시하는 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftRecipeEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 레시피 카드 표시 데이터를 받아 UI를 갱신
	void SetRecipeEntryData(const FEDCraftRecipeEntryDisplayData& InDisplayData);

	// 현재 카드가 선택된 상태인지 시각적으로 표시
	void SetSelectedState(bool bSelected);

	// 이 카드가 가리키는 레시피 Row 이름을 반환
	FName GetRecipeRowId() const { return RecipeRowId; }

	// 레시피 카드 클릭 이벤트
	FOnEDCraftRecipeEntryClicked OnRecipeEntryClicked;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:
	// 결과 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> RecipeIconImage;

	// 결과 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> RecipeNameText;

	// 제작 가능 여부 상태 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> CraftStateText;

	// 희귀도 강조색 영역
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> RarityAccent;

	// 선택 상태 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> SelectionBorder;

private:
	// 현재 카드가 가리키는 레시피 Row 이름
	FName RecipeRowId = NAME_None;
};
