// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "EDMatchResultWidget.generated.h"

class UTextBlock;
class UWidgetSwitcher;

/**
 * 매치 종료 시 Modal 레이어에 올라가는 결과 위젯.
 * UIManager의 OpenPanel(Panel_MatchResult)로 열린다.
 *
 * 서버에서 계산된 팀 등수 배열(index 0 = 1등)과 로컬 플레이어의 TeamId를
 * SetResult로 전달받아 승리/패배 분기 + 내 팀 등수만 표시한다.
 *
 * 승/패 분기는 ResultSwitcher(Index 0=Victory, 1=Defeat)로 처리.
 * 내 순위는 MyRankText에 "#N" 형태로 세팅.
 */
UCLASS()
class ETERNALDREAMS_API UEDMatchResultWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/**
	 * @param TeamRankings  index 0 = 1등 팀 ID, index 1 = 2등, ...
	 * @param MyTeamId      로컬 플레이어의 팀 ID. 1등과 같으면 승리.
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|Match")
	void SetResult(const TArray<int32>& TeamRankings, int32 MyTeamId);

protected:
	/** BP에서 추가 연출(애니 등) 구성. */
	UFUNCTION(BlueprintImplementableEvent, Category = "ED|Match")
	void OnResultSet(bool bIsVictory, int32 MyRank);

	/** 승/패 분기용 WidgetSwitcher. Index 0 = Victory, Index 1 = Defeat */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ResultSwitcher;

	/** 내 팀 등수 텍스트. "#1" 형태로 세팅 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MyRankText;
};
