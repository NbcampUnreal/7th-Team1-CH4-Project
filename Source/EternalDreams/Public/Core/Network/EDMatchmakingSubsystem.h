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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRegisterResult, bool, bSuccess, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchQueued);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFound, const FString&, MatchId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyStateChanged, const FString&, JsonState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyZoneSelected, int32, ZoneId);
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

	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void Register(const FString& LoginId, const FString& Password, const FString& InNickname);

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

	/**
	 * 로비/리스폰 존 선택.
	 * 현재 IOCP 서버에 Zone OpCode가 없으므로 로컬 상태(Subsystem)만 갱신하고 델리게이트로 브로드캐스트한다.
	 * 서버 측 핸들러가 추가되면 이 함수에서 패킷도 같이 전송하도록 확장할 것.
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|Matchmaking")
	void LobbySelectZone(int32 ZoneId);

	UFUNCTION(BlueprintPure, Category = "ED|Matchmaking")
	int32 GetSelectedZoneId() const { return CurrentZoneId; }

	// -------------------------------------------------------
	//  Delegates
	// -------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnLoginResult OnLoginResult;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnRegisterResult OnRegisterResult;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnMatchQueued OnMatchQueued;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnMatchFound OnMatchFound;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnLobbyStateChanged OnLobbyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnLobbyZoneSelected OnLobbyZoneSelected;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnGameStart OnGameStart;

	UPROPERTY(BlueprintAssignable, Category = "ED|Matchmaking")
	FOnMatchServerDisconnected OnMatchServerDisconnected;

private:
	void HandlePacket(uint16 OpCode, const FString& JsonBody);
	void HandleDisconnect();

	// Packet handlers
	void HandleLoginRes(const FString& JsonBody);
	void HandleRegisterRes(const FString& JsonBody);
	void HandleMatchQueueRes(const FString& JsonBody);
	void HandleMatchFound(const FString& JsonBody);
	void HandleLobbyState(const FString& JsonBody);
	void HandleGameStart(const FString& JsonBody);

	FEDTCPClient* TCPClient;

	FString AuthToken;
	FString Nickname;
	FString CurrentMatchId;
	int32   CurrentZoneId = 0;
};
