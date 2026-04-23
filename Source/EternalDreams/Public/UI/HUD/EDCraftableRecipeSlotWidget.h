#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDCraftableRecipeSlotWidget.generated.h"

class UBorder;
class UImage;
class USizeBox;
class UTexture2D;

struct FEDCraftableRecipeSlotDisplayData
{
	TObjectPtr<UTexture2D> IconTexture = nullptr;
	EEDItemRarity Rarity = EEDItemRarity::Normal;
	bool bIsCurrentCraftTarget = false;
};

/**
 * 제작 가능 아이템 바에 표시되는 단일 슬롯 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftableRecipeSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯 표시 데이터를 받아 아이콘과 강조 상태를 갱신
	void SetSlotDisplayData(const FEDCraftableRecipeSlotDisplayData& InDisplayData);

protected:
	// 슬롯 전체 크기를 고정하는 SizeBox
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<USizeBox> SlotSizeBox;

	// 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> ItemIconImage;

	// 현재 제작 대상일 때 표시하는 강조 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> CurrentTargetBorder;
};
