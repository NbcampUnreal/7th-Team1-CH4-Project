// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameMode.h"
#include "EDGameMode.generated.h"

UCLASS()
class ETERNALDREAMS_API AEDGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AEDGameMode();

	/** BP_GameMode에서 설정할 게임맵 경로 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ED|GameMode")
	FString GameMapPath = TEXT("/Game/ED/Asset/IgnoredAsset/Test/GameMap");

	/** 서버에서 호출. 모든 플레이어를 GameMap으로 이동시킨다. */
	UFUNCTION(BlueprintCallable, Category = "ED|GameMode")
	void StartGame();

	/** 페이즈 전환 (서버 전용) */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void SetPhase(FGameplayTag NewPhase);

	/** 현재 페이즈 확인 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	FGameplayTag GetCurrentPhase() const;

	/** 페이즈 시퀀스 시작 (첫 페이즈부터 타이머 자동 진행) */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void StartPhaseSequence();

	/** 현재 페이즈 스킵 → 다음 페이즈로 즉시 전환 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void SkipToNextPhase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// -------------------------------------------------------
	// 페이즈 설정
	// -------------------------------------------------------

	/** 페이즈 순서 (에디터에서 설정). 비어있으면 자동 전환 비활성화 */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	TArray<FGameplayTag> PhaseSequence;

	/** 각 페이즈 지속 시간 (초). PhaseSequence와 1:1 매칭. 기본 90초 */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	TArray<float> PhaseDurations;

	/** PhaseDurations 미설정 시 사용할 기본 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	float DefaultPhaseDuration = 90.f;

private:
	/** 현재 페이즈 인덱스 (PhaseSequence 기준) */
	int32 CurrentPhaseIndex = INDEX_NONE;

	/** 현재 페이즈 남은 시간 */
	float PhaseTimer = 0.f;

	/** 페이즈 시퀀스 진행 중 여부 */
	bool bPhaseSequenceActive = false;

	/** 지정 인덱스의 페이즈로 전환 */
	void AdvanceToPhase(int32 PhaseIndex);

	/** 현재 인덱스의 지속 시간 반환 */
	float GetPhaseDuration(int32 PhaseIndex) const;
};
