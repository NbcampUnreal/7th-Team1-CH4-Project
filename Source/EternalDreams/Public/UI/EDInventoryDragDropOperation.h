#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventoryDragDropOperation.generated.h"

class UEDInventoryComponent;
class UTexture2D;

UCLASS()
class ETERNALDREAMS_API UEDInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 드래그 시작 슬롯 인덱스
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 SourceSlotIndex = INDEX_NONE;

	// 드래그 시작 인벤토리 컴포넌트
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> SourceInventoryComponent = nullptr;

	// 드래그 비주얼에 표시할 아이템 이름
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	FText SourceItemName;

	// 드래그 비주얼에 표시할 아이템 수량
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 SourceItemQuantity = 0;

	// 드래그 비주얼에 표시할 아이템 희귀도
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	EEDItemRarity SourceItemRarity = EEDItemRarity::Normal;

	// 드래그 비주얼에 표시할 아이템 아이콘
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UTexture2D> SourceIconTexture = nullptr;

	// 드롭 타겟이 정상적으로 처리했는지 여부
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	bool bHandledByDropTarget = false;
};
