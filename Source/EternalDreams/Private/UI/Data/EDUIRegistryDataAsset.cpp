// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Data/EDUIRegistryDataAsset.h"

#include "Blueprint/UserWidget.h"

const FEDUIRegistryEntry* UEDUIRegistryDataAsset::FindEntryById(FName InWidgetId) const
{
	// 잘못된 ID 요청 방지
	if (InWidgetId.IsNone())
	{
		return nullptr;
	}

	for (const FEDUIRegistryEntry& Entry : Entries)
	{
		if (Entry.WidgetId == InWidgetId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool UEDUIRegistryDataAsset::ValidateEntries() const
{
	TSet<FName> UsedIds;

	for (const FEDUIRegistryEntry& Entry : Entries)
	{
		// ID가 비어 있으면 잘못된 데이터
		if (Entry.WidgetId.IsNone())
		{
			UE_LOG(LogTemp, Warning, TEXT("EDUIRegistryDataAsset: 비어 있는 WidgetId가 있습니다."));
			return false;
		}

		// 위젯 클래스가 비어 있으면 생성 불가
		if (!Entry.WidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("EDUIRegistryDataAsset: WidgetClass가 비어 있습니다. WidgetId = %s"),
			       *Entry.WidgetId.ToString());
			return false;
		}

		// 중복 WidgetId 방지
		if (UsedIds.Contains(Entry.WidgetId))
		{
			UE_LOG(LogTemp, Warning, TEXT("EDUIRegistryDataAsset: 중복된 WidgetId가 있습니다. WidgetId = %s"),
			       *Entry.WidgetId.ToString());
			return false;
		}

		UsedIds.Add(Entry.WidgetId);
	}

	return true;
}
