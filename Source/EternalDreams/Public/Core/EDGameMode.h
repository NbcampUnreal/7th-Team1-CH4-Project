// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameMode.h"
#include "EDGameMode.generated.h"

/**
 * In-game GameMode.
 * Phase logic lives here and late join is blocked once the match has started.
 */
UCLASS()
class ETERNALDREAMS_API AEDGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AEDGameMode();

	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void SetPhase(FGameplayTag NewPhase);

	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	FGameplayTag GetCurrentPhase() const;

	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void StartPhaseSequence();

	UFUNCTION(BlueprintCallable, Category = "ED|Phase")
	void SkipToNextPhase();

protected:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	TArray<FGameplayTag> PhaseSequence;

	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	TArray<float> PhaseDurations;

	UPROPERTY(EditDefaultsOnly, Category = "ED|Phase")
	float DefaultPhaseDuration = 90.f;

private:
	int32 CurrentPhaseIndex = INDEX_NONE;
	float PhaseTimer = 0.f;
	bool bPhaseSequenceActive = false;

	void AdvanceToPhase(int32 PhaseIndex);
	float GetPhaseDuration(int32 PhaseIndex) const;
};
