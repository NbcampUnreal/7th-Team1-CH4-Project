// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameState.h"
#include "EDGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePhaseChanged, const FGameplayTag&, OldPhase, const FGameplayTag&, NewPhase);

UCLASS()
class ETERNALDREAMS_API AEDGameState : public AGameState
{
	GENERATED_BODY()

public:
	AEDGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

	// -------------------------------------------------------
	// Phase 정보 (읽기)
	// -------------------------------------------------------

	/** 현재 게임 페이즈 태그 반환 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	FGameplayTag GetCurrentPhase() const { return CurrentPhase; }

	/** 현재 페이즈 남은 시간 (초) */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	float GetPhaseRemainingTime() const { return PhaseRemainingTime; }

	/** 현재 몇 일차인지 (1~4, 0=미시작). CurrentPhase 태그에서 파싱 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	int32 GetCurrentDay() const;

	/** 현재 밤인지. CurrentPhase 태그에서 파싱 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	bool GetIsNight() const;

	// -------------------------------------------------------
	// Phase 정보 (서버에서만 호출)
	// -------------------------------------------------------

	void SetCurrentPhase(const FGameplayTag& NewPhase);
	void SetPhaseRemainingTime(float NewTime);

	// -------------------------------------------------------
	// 델리게이트
	// -------------------------------------------------------

	/** 페이즈 변경 시 브로드캐스트 */
	UPROPERTY(BlueprintAssignable, Category = "ED|Phase")
	FOnGamePhaseChanged OnGamePhaseChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase, VisibleAnywhere, BlueprintReadOnly, Category = "ED|Phase")
	FGameplayTag CurrentPhase;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "ED|Phase")
	float PhaseRemainingTime = 0.f;

	UFUNCTION()
	void OnRep_CurrentPhase(const FGameplayTag& OldPhase);
};
