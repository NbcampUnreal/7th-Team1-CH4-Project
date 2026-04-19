#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDCraftableRecipeSlotWidget.generated.h"

class UBorder;
class UImage;
class UTexture2D;

struct FEDCraftableRecipeSlotDisplayData
{
	TObjectPtr<UTexture2D> IconTexture = nullptr;
	EEDItemRarity Rarity = EEDItemRarity::Normal;
	bool bIsCurrentCraftTarget = false;
};

/**
 * 제작 가능 아이템 바에서 아이템 하나를 표시하는 슬롯 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftableRecipeSlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 슬롯 표시 데이터를 받아 아이콘과 강조 상태를 갱신
	void SetSlotDisplayData(const FEDCraftableRecipeSlotDisplayData& InDisplayData);

protected:
	// 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> ItemIconImage;

	// 현재 제작 대상일 때 표시할 강조 테두리
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UBorder> CurrentTargetBorder;
};
