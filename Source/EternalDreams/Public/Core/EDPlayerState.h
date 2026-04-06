// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EDPlayerState.generated.h"

UCLASS()
class ETERNALDREAMS_API AEDPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AEDPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	// -------------------------------------------------------
	// 팀 / 준비 상태
	// -------------------------------------------------------

	/** 팀 ID (-1 = 미배정, 0 = TeamA, 1 = TeamB) */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	int32 TeamId = -1;

	/** 준비 완료 여부 */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "ED|Player")
	bool bReady = false;
};

