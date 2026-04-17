#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDCraftingInteractionComponent.generated.h"

class APlayerController;
class UEDItemCraftingWidget;

/**
 * 플레이어의 제작 입력과 제작 HUD 위젯 사이를 중개하는 컴포넌트
 * 컨트롤러는 입력만 전달하고, 실제 위젯 탐색과 제작 요청은 이 컴포넌트가 담당
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UEDCraftingInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEDCraftingInteractionComponent();

	// 제작 입력을 처리하고, 현재 선택된 레시피 제작 시도
	void HandleCraftInput();

private:
	// 소유 플레이어 컨트롤러 반환
	APlayerController* GetOwningPlayerController() const;

	// 현재 화면에 떠 있는 제작 위젯 인스턴스를 찾음
	UEDItemCraftingWidget* FindCraftingWidget() const;
};
