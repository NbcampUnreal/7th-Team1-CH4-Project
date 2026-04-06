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

	/** 현재 게임 페이즈 태그 반환 */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	FGameplayTag GetCurrentPhase() const { return CurrentPhase; }

	/** 현재 페이즈 남은 시간 (초) */
	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	float GetPhaseRemainingTime() const { return PhaseRemainingTime; }

	/** 페이즈 변경 (서버에서만 호출) */
	void SetCurrentPhase(const FGameplayTag& NewPhase);

	/** 남은 시간 갱신 (서버에서만 호출) */
	void SetPhaseRemainingTime(float NewTime);

	/** 페이즈 변경 시 브로드캐스트 델리게이트 */
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
