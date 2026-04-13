// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EDMatchmakingSubsystem.generated.h"

class FEDTCPClient;

// ============================================================
//  Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoginResult, bool, bSuccess, const FString&, Nickname, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchQueued);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFound, const FString&, MatchId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyStateChanged, const FString&, JsonState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameStart, const FString&, ServerIP, int32, ServerPort);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchServerDisconnected);

/**
 * UEDMatchmakingSubsystem
 *
 * GameInstance Subsystem that manages communication with EDMatchServer.
 * Provides Blueprint-callable API for login, matchmaking, lobby.
 */
UCLASS()
class ETERNALDREAMS_API UEDMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEDMatchmakingSubsystem();
	virtual ~UEDMatchmakingSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// -------------------------------------------------------
	//  Connection
	// -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	bool ConnectToMatchServer(const FString& Host = TEXT("127.0.0.1"), int32 Port = 9000);

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void DisconnectFromMatchServer();

	UFUNCTION(BlueprintPure, Category = "ED|Matchmaking")
	bool IsConnectedToMatchServer() const;

	// -------------------------------------------------------
	//  Login
	// -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void Login(const FString& LoginId, const FString& Password);

	UFUNCTION(BlueprintPure, Category = "ED|Matchmaking")
	bool IsLoggedIn() const { return !AuthToken.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "ED|Matchmaking")
	FString GetNickname() const { return Nickname; }

	UFUNCTION(BlueprintPure, Category = "ED|Matchmaking")
	FString GetAuthToken() const { return AuthToken; }

	// -------------------------------------------------------
	//  Matchmaking (Phase 5)
	// -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void RequestMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void CancelMatchmaking();

	// -------------------------------------------------------
	//  Lobby (Phase 6)
	// -------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void LobbySetReady();

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void LobbyChangeTeam(int32 NewTeamId);

	// -------------------------------------------------------
	//  Delegates
	// -------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnLoginResult OnLoginResult;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnMatchQueued OnMatchQueued;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnMatchFound OnMatchFound;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnLobbyStateChanged OnLobbyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnGameStart OnGameStart;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnMatchServerDisconnected OnMatchServerDisconnected;

private:
	void HandlePacket(uint16 OpCode, const FString& JsonBody);
	void HandleDisconnect();

	// Packet handlers
	void HandleLoginRes(const FString& JsonBody);
	void HandleMatchQueueRes(const FString& JsonBody);
	void HandleMatchFound(const FString& JsonBody);
	void HandleLobbyState(const FString& JsonBody);
	void HandleGameStart(const FString& JsonBody);

	FEDTCPClient* TCPClient;

	FString AuthToken;
	FString Nickname;
	FString CurrentMatchId;
};
