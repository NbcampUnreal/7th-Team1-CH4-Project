// Copyright Eternal Dreams Team. All Rights Reserved.

#include "UI/HUD/EDMatchResultWidget.h"

#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

void UEDMatchResultWidget::SetResult(const TArray<int32>& TeamRankings, int32 MyTeamId)
{
	// 내 팀의 등수 검색 (1부터 시작, 못 찾으면 INDEX_NONE)
	int32 MyRank = INDEX_NONE;
	for (int32 i = 0; i < TeamRankings.Num(); ++i)
	{
		if (TeamRankings[i] == MyTeamId)
		{
			MyRank = i + 1;
			break;
		}
	}

	const bool bIsVictory = (MyRank == 1);

	if (ResultSwitcher)
	{
		ResultSwitcher->SetActiveWidgetIndex(bIsVictory ? 0 : 1);
	}

	if (MyRankText && MyRank != INDEX_NONE)
	{
		MyRankText->SetText(FText::FromString(FString::Printf(TEXT("#%d"), MyRank)));
	}

	OnResultSet(bIsVictory, MyRank);
}
