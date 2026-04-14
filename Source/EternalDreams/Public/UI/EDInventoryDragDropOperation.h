#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "EDInventoryDragDropOperation.generated.h"

UCLASS()
class ETERNALDREAMS_API UEDInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 드래그 시작 슬롯 인덱스
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 SourceSlotIndex = INDEX_NONE;
};
