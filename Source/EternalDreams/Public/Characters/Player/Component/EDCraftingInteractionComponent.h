#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDCraftingInteractionComponent.generated.h"

class APlayerController;
class UEDInventoryComponent;
struct FEDCraftableRecipeEntry;

/**
 * 플레이어의 제작 입력을 처리하는 컴포넌트
 * 현재 제작 가능한 첫 번째 레시피를 보여줌
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UEDCraftingInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEDCraftingInteractionComponent();

	// 제작 입력을 처리하고 현재 제작 가능한 첫 번째 레시피 제작을 시도
	void HandleCraftInput();

private:
	// 소유 플레이어 컨트롤러 반환
	APlayerController* GetOwningPlayerController() const;

	// 플레이어가 사용하는 인벤토리 컴포넌트 반환
	UEDInventoryComponent* GetOwningInventoryComponent() const;

	// 현재 제작 가능한 레시피 캐시 중 첫 번째 항목을 반환
	bool TryGetFirstCraftableRecipeEntry(UEDInventoryComponent* InventoryComponent, FEDCraftableRecipeEntry& OutRecipeEntry) const;
};
