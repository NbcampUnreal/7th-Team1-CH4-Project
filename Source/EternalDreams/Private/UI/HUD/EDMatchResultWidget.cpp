// Copyright Eternal Dreams Team. All Rights Reserved.

#include "UI/HUD/EDMatchResultWidget.h"

#include "Components/TextBlock.h"
#include "Core/EDPlayerState.h"

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

	if (ResultTitleText)
	{
		FText Title;
		if (bIsVictory)
		{
			Title = NSLOCTEXT("EDMatch", "Victory", "승리!");
		}
		else if (MyRank == INDEX_NONE)
		{
			Title = NSLOCTEXT("EDMatch", "NoContest", "무승부");
		}
		else
		{
			Title = NSLOCTEXT("EDMatch", "Defeat", "패배");
		}
		ResultTitleText->SetText(Title);
	}

	OnResultSet(bIsVictory, MyRank, TeamRankings);
}
