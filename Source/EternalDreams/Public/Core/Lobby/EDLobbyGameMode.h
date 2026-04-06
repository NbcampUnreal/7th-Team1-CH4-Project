// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EDLobbyGameMode.generated.h"

/**
 * Lobby-only GameMode.
 * Players join here, toggle Ready, and the server moves everyone to the game map.
 */
UCLASS()
class ETERNALDREAMS_API AEDLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AEDLobbyGameMode();

	/** Destination map for the actual match. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ED|Lobby")
	FString GameMapPath = TEXT("/Game/ED/Asset/IgnoredAsset/Test/GameMap");

	/** Checks all connected players and starts the match on the server when ready. */
	UFUNCTION(BlueprintCallable, Category = "ED|Lobby")
	void TryStartGame();

protected:
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	int32 GetTeamWithFewerPlayers() const;

	// Prevent duplicate travel when multiple Ready updates arrive close together.
	bool bGameStarting = false;
};
