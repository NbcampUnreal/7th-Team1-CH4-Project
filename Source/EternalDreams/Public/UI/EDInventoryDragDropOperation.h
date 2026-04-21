#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "EDInventoryDragDropOperation.generated.h"

class UEDInventoryComponent;

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
};
