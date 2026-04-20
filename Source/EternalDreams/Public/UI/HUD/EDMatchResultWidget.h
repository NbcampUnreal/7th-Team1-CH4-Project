// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDMatchResultWidget.generated.h"

class UTextBlock;

/**
 * 매치 종료 시 Modal 레이어에 올라가는 결과 위젯.
 * UIManager의 OpenPanel(Panel_MatchResult)로 열린다.
 *
 * 서버에서 계산된 팀 등수 배열(index 0 = 1등)과 로컬 플레이어의 TeamId를
 * SetResult로 전달받아 승리/패배 + 등수 순위를 표시한다.
 *
 * 세부 레이아웃(등수 리스트, 팀 이름 색상 등)은 BP의 OnResultSet 이벤트에서 구성.
 */
UCLASS()
class ETERNALDREAMS_API UEDMatchResultWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * @param TeamRankings  index 0 = 1등 팀 ID, index 1 = 2등 팀 ID, ...
	 * @param MyTeamId      로컬 플레이어의 팀 ID. 1등과 같으면 승리.
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|Match")
	void SetResult(const TArray<int32>& TeamRankings, int32 MyTeamId);

protected:
	/** BP에서 등수 리스트 레이아웃 및 연출 구성. C++은 제목 텍스트만 세팅. */
	UFUNCTION(BlueprintImplementableEvent, Category = "ED|Match")
	void OnResultSet(bool bIsVictory, int32 MyRank, const TArray<int32>& TeamRankings);

	/** 승리/패배/무승부 제목 텍스트 (Optional: BP에 없으면 스킵) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ResultTitleText;
};
