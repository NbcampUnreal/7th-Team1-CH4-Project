#pragma once

#include "CoreMinimal.h"
#include "UObject/PrimaryAssetId.h"
#include "EDItemTypes.generated.h"

UENUM(BlueprintType)
enum class EEDInventoryItemType : uint8
{
	Equippable,
	Consumable,
	Ingredient,
	Skill
};

UENUM(BlueprintType)
enum class EEDEquippableType : uint8
{
	None,
	Weapon,
	TopArmor,
	BottomArmor
};

UENUM(BlueprintType)
enum class EEDItemRarity : uint8
{
	Normal,
	Rare,
	Epic,
	Legendary,
	Unique
};

USTRUCT(BlueprintType)
struct ETERNALDREAMS_API FEDInventoryItemHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FPrimaryAssetId ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
	int32 Quantity = 0;

	bool IsValid() const
	{
		return ItemId.IsValid() && Quantity > 0;
	}
};
