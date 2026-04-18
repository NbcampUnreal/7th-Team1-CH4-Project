#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDCraftTreeNodeWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UTexture2D;

// 제작 트리 노드 위젯이 화면에 그리기 위해 사용하는 표시 데이터
struct FEDCraftTreeNodeDisplayData
{
	FText DisplayName;
	TObjectPtr<UTexture2D> IconTexture = nullptr;
	EEDItemRarity Rarity = EEDItemRarity::Normal;
	int32 RequiredQuantity = 1;
};

/**
 * 제작 트리에서 재료 또는 결과 아이템 하나를 표시하는 노드 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftTreeNodeWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 노드 표시 데이터를 받아 UI를 갱신
	void SetTreeNodeDisplayData(const FEDCraftTreeNodeDisplayData& InDisplayData);

protected:
	// 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> ItemIconImage;

	// 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> ItemNameText;

	// 필요 수량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> QuantityText;

	// 희귀도 강조색 영역
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> RarityAccent;
};
