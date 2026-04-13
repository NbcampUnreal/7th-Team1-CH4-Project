// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EDDediServerSubsystem.generated.h"

class FEDTCPClient;

/**
 * UEDDediServerSubsystem
 *
 * Dedicated Server only subsystem.
 * Connects to EDMatchServer (IOCP) for:
 *  - DEDI_REGISTER: register this server in the pool
 *  - DEDI_ASSIGN_MATCH: receive match assignment with player auth tokens
 *  - DEDI_HEARTBEAT: periodic status update
 *  - DEDI_MATCH_RESULT: report match results
 *
 * Also provides PreLogin token verification for connecting players.
 */
UCLASS()
class ETERNALDREAMS_API UEDDediServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEDDediServerSubsystem();
	virtual ~UEDDediServerSubsystem();

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -------------------------------------------------------
	//  Connection to IOCP MatchServer
	// -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ED|DediServer")
	bool ConnectToMatchServer(const FString& Host = TEXT("127.0.0.1"), int32 Port = 9000);

	UFUNCTION(BlueprintCallable, Category = "ED|DediServer")
	void RegisterSelf(const FString& IP, int32 GamePort = 7777, int32 MaxPlayers = 6);

	// -------------------------------------------------------
	//  Auth Token Verification (for PreLogin)
	// -------------------------------------------------------

	/** Check if a token is valid for this match */
	bool IsTokenAuthorized(const FString& Token) const;

	/** Get current match ID */
	FString GetCurrentMatchId() const { return CurrentMatchId; }

	/** Is this server assigned to a match? */
	bool IsMatchAssigned() const { return !CurrentMatchId.IsEmpty(); }

	// -------------------------------------------------------
	//  Match Result Reporting
	// -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ED|DediServer")
	void ReportMatchResult(const FString& ResultJson);

private:
	void HandlePacket(uint16 OpCode, const FString& JsonBody);
	void HandleDisconnect();

	void HandleDediAssignMatch(const FString& JsonBody);

	void SendHeartbeat();

	FEDTCPClient* TCPClient;
	FTimerHandle HeartbeatTimerHandle;

	FString CurrentMatchId;
	TArray<FString> AuthorizedTokens;
	FString ServerStatus; // "idle", "ingame"
};
