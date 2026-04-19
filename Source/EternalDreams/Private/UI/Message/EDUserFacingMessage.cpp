// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Message/EDUserFacingMessage.h"

#include "Internationalization/Text.h"

namespace EDUserFacingMessage
{
namespace Craft
{
FText GetFailureText(EEDInventoryActionFailure Failure)
{
	switch (Failure)
	{
	case EEDInventoryActionFailure::None:
		return FText::GetEmpty();
	case EEDInventoryActionFailure::InvalidInventory:
		return NSLOCTEXT("Craft", "UserFailureInvalidInventory", "제작 정보를 찾을 수 없습니다.");
	case EEDInventoryActionFailure::InvalidSlot:
		return NSLOCTEXT("Craft", "UserFailureInvalidSlot", "잘못된 제작 대상입니다.");
	case EEDInventoryActionFailure::EmptySlot:
		return NSLOCTEXT("Craft", "UserFailureEmptySlot", "제작할 아이템이 없습니다.");
	case EEDInventoryActionFailure::InvalidQuantity:
		return NSLOCTEXT("Craft", "UserFailureInvalidQuantity", "제작 수량 정보가 올바르지 않습니다.");
	case EEDInventoryActionFailure::SlotConflict:
		return NSLOCTEXT("Craft", "UserFailureSlotConflict", "제작 상태가 올바르지 않습니다.");
	case EEDInventoryActionFailure::NoSpace:
		return NSLOCTEXT("Craft", "UserFailureNoSpace", "인벤토리 공간이 부족해 제작할 수 없습니다.");
	case EEDInventoryActionFailure::StackLimit:
		return NSLOCTEXT("Craft", "UserFailureStackLimit", "더 이상 같은 아이템을 제작할 수 없습니다.");
	case EEDInventoryActionFailure::MissingData:
		return NSLOCTEXT("Craft", "UserFailureMissingData", "아이템 정보를 찾을 수 없습니다.");
	case EEDInventoryActionFailure::InvalidRecipe:
		return NSLOCTEXT("Craft", "UserFailureInvalidRecipe", "유효하지 않은 제작식입니다.");
	case EEDInventoryActionFailure::MissingIngredient:
		return NSLOCTEXT("Craft", "UserFailureMissingIngredient", "재료가 부족해 제작할 수 없습니다.");
	case EEDInventoryActionFailure::NotConsumable:
		return NSLOCTEXT("Craft", "UserFailureNotConsumable", "사용할 수 없는 아이템입니다.");
	case EEDInventoryActionFailure::HealthAlreadyFull:
		return NSLOCTEXT("Craft", "UserFailureHealthAlreadyFull", "이미 체력이 가득 찬 상태입니다.");
	case EEDInventoryActionFailure::EffectApplyFailed:
		return NSLOCTEXT("Craft", "UserFailureEffectApplyFailed", "효과 적용에 실패했습니다.");
	default:
		return NSLOCTEXT("Craft", "UserFailureGeneric", "알 수 없는 이유로 제작에 실패했습니다.");
	}
}

FText GetSuccessText(const FText& ResultItemName)
{
	return FText::Format(
		NSLOCTEXT("Craft", "UserSuccess", "{0} 제작에 성공했습니다."),
		ResultItemName);
}
}

namespace Inventory
{
FText GetFailureText(EEDInventoryActionFailure Failure)
{
	switch (Failure)
	{
	case EEDInventoryActionFailure::None:
		return FText::GetEmpty();
	case EEDInventoryActionFailure::InvalidInventory:
		return NSLOCTEXT("InventoryUI", "UserFailureInvalidInventory", "인벤토리 정보를 찾을 수 없습니다.");
	case EEDInventoryActionFailure::InvalidSlot:
		return NSLOCTEXT("InventoryUI", "UserFailureInvalidSlot", "잘못된 슬롯입니다.");
	case EEDInventoryActionFailure::EmptySlot:
		return NSLOCTEXT("InventoryUI", "UserFailureEmptySlot", "선택한 슬롯이 비어 있습니다.");
	case EEDInventoryActionFailure::InvalidQuantity:
		return NSLOCTEXT("InventoryUI", "UserFailureInvalidQuantity", "수량 정보가 올바르지 않습니다.");
	case EEDInventoryActionFailure::SlotConflict:
		return NSLOCTEXT("InventoryUI", "UserFailureSlotConflict", "슬롯 상태가 충돌합니다.");
	case EEDInventoryActionFailure::NoSpace:
		return NSLOCTEXT("InventoryUI", "UserFailureNoSpace", "인벤토리 공간이 부족합니다.");
	case EEDInventoryActionFailure::StackLimit:
		return NSLOCTEXT("InventoryUI", "UserFailureStackLimit", "더 이상 같은 아이템을 넣을 수 없습니다.");
	case EEDInventoryActionFailure::MissingData:
		return NSLOCTEXT("InventoryUI", "UserFailureMissingData", "아이템 데이터가 없습니다.");
	case EEDInventoryActionFailure::InvalidRecipe:
		return NSLOCTEXT("InventoryUI", "UserFailureInvalidRecipe", "유효하지 않은 레시피입니다.");
	case EEDInventoryActionFailure::MissingIngredient:
		return NSLOCTEXT("InventoryUI", "UserFailureMissingIngredient", "재료가 부족하여 제작할 수 없습니다.");
	case EEDInventoryActionFailure::NotConsumable:
		return NSLOCTEXT("InventoryUI", "UserFailureNotConsumable", "사용할 수 없는 아이템입니다.");
	case EEDInventoryActionFailure::HealthAlreadyFull:
		return NSLOCTEXT("InventoryUI", "UserFailureHealthAlreadyFull", "이미 체력이 가득 찬 상태입니다.");
	case EEDInventoryActionFailure::EffectApplyFailed:
		return NSLOCTEXT("InventoryUI", "UserFailureEffectApplyFailed", "효과 적용에 실패했습니다.");
	default:
		return NSLOCTEXT("InventoryUI", "UserFailureGeneric", "알 수 없는 이유로 인벤토리 작업에 실패했습니다.");
	}
}

FText GetSuccessText(const FText& TargetName, const FText& ActionName)
{
	return FText::Format(
		NSLOCTEXT("InventoryUI", "UserSuccess", "{0} {1}에 성공했습니다."),
		TargetName,
		ActionName);
}
}
}
