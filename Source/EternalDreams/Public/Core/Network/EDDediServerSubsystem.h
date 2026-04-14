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
	//  Player Info (from IOCP DEDI_ASSIGN)
	// -------------------------------------------------------

	/** IOCP에서 받은 플레이어 정보 */
	struct FAssignedPlayerInfo
	{
		FString Nickname;
		int32 TeamId = -1;
	};

	/** 토큰으로 플레이어 팀/닉네임 조회. 없으면 nullptr */
	const FAssignedPlayerInfo* GetPlayerInfoByToken(const FString& Token) const;

	/** 매치에 할당된 총 플레이어 수 */
	int32 GetExpectedPlayerCount() const { return PlayerInfoMap.Num(); }

	// -------------------------------------------------------
	//  PreLogin → PostLogin Token Bridging
	// -------------------------------------------------------

	/** PreLogin에서 Address→Token 매핑 저장 (EDGameMode에서 호출) */
	void StorePendingToken(const FString& Address, const FString& Token) { PendingTokenMap.Add(Address, Token); }

	/** PostLogin에서 Address로 토큰 조회 후 제거 */
	FString ConsumePendingToken(const FString& Address);

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

	/** 토큰 → 플레이어 정보 매핑 (DEDI_ASSIGN에서 수신) */
	TMap<FString, FAssignedPlayerInfo> PlayerInfoMap;

	/** PreLogin에서 검증 성공한 토큰을 UniqueNetId 문자열과 매핑 (PostLogin에서 조회용) */
	TMap<FString, FString> PendingTokenMap; // Key: Address or UniqueNetId string, Value: Token
};
