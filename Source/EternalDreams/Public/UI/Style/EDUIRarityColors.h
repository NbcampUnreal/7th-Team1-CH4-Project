#pragma once

#include "CoreMinimal.h"
#include "Item/Core/EDItemTypes.h"

namespace EDRarityColors
{
	inline FLinearColor Resolve(EEDItemRarity InRarity)
	{
		switch (InRarity)
		{
		case EEDItemRarity::Rare:
			return FLinearColor(0.20f, 0.45f, 1.00f, 1.00f);
		case EEDItemRarity::Epic:
			return FLinearColor(0.65f, 0.25f, 1.00f, 1.00f);
		case EEDItemRarity::Legendary:
			return FLinearColor(1.00f, 0.55f, 0.10f, 1.00f);
		case EEDItemRarity::Unique:
			return FLinearColor(1.00f, 0.20f, 0.20f, 1.00f);
		case EEDItemRarity::Normal:
		default:
			return FLinearColor(0.65f, 0.65f, 0.65f, 1.00f);
		}
	}
}
