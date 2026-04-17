#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/EDInventoryTypes.h"

namespace EDUserFacingMessage
{
	namespace Craft
	{
		// 제작 실패 사유를 사용자에게 보여줄 문구로 변환
		ETERNALDREAMS_API FText GetFailureText(EEDInventoryActionFailure Failure);

		// 제작 성공 시 결과 아이템 이름을 포함한 문구를 만듦
		ETERNALDREAMS_API FText GetSuccessText(const FText& ResultItemName);
	}

	namespace Inventory
	{
		// 인벤토리 액션 실패 사유를 사용자에게 보여줄 문구로 변환
		ETERNALDREAMS_API FText GetFailureText(EEDInventoryActionFailure Failure);

		// 인벤토리 액션 성공 시 대상 이름과 동작 이름을 합친 문구를 만듦
		ETERNALDREAMS_API FText GetSuccessText(const FText& TargetName, const FText& ActionName);
	}
}
