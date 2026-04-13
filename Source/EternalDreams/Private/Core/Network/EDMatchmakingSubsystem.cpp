// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Network/EDMatchmakingSubsystem.h"
#include "Core/Network/EDTCPClient.h"
#include "Core/Network/EDNetProtocol.h"
#include "Core/EDGameInstance.h"
#include "EternalDreams.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// ============================================================
//  Lifecycle
// ============================================================

UEDMatchmakingSubsystem::UEDMatchmakingSubsystem()
	: TCPClient(nullptr)
{
}

UEDMatchmakingSubsystem::~UEDMatchmakingSubsystem()
{
	DisconnectFromMatchServer();
}

void UEDMatchmakingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Initialized"));
}

void UEDMatchmakingSubsystem::Deinitialize()
{
	DisconnectFromMatchServer();
	Super::Deinitialize();
}

// ============================================================
//  Connection
// ============================================================

bool UEDMatchmakingSubsystem::ConnectToMatchServer(const FString& Host, int32 Port)
{
	if (TCPClient && TCPClient->IsConnected())
	{
		UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Already connected"));
		return true;
	}

	TCPClient = new FEDTCPClient();

	TCPClient->OnPacketReceived.BindUObject(this, &UEDMatchmakingSubsystem::HandlePacket);
	TCPClient->OnDisconnected.BindUObject(this, &UEDMatchmakingSubsystem::HandleDisconnect);

	if (!TCPClient->Connect(Host, Port))
	{
		UE_LOG(LogEDCore, Error, TEXT("[MatchmakingSubsystem] Connect failed"));
		delete TCPClient;
		TCPClient = nullptr;
		return false;
	}

	return true;
}

void UEDMatchmakingSubsystem::DisconnectFromMatchServer()
{
	if (TCPClient)
	{
		TCPClient->Disconnect();
		delete TCPClient;
		TCPClient = nullptr;
	}

	AuthToken.Empty();
	Nickname.Empty();
	CurrentMatchId.Empty();
}

bool UEDMatchmakingSubsystem::IsConnectedToMatchServer() const
{
	return TCPClient && TCPClient->IsConnected();
}

// ============================================================
//  Login
// ============================================================

void UEDMatchmakingSubsystem::Login(const FString& LoginId, const FString& Password)
{
	if (!IsConnectedToMatchServer())
	{
		UE_LOG(LogEDCore, Error, TEXT("[MatchmakingSubsystem] Not connected"));
		OnLoginResult.Broadcast(false, TEXT(""), TEXT("not_connected"));
		return;
	}

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("id"), LoginId);
	Json->SetStringField(TEXT("pw"), Password);

	FString Body;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	TCPClient->SendPacket(EDNet::C2S_LOGIN_REQ, Body);
}

// ============================================================
//  Matchmaking
// ============================================================

void UEDMatchmakingSubsystem::RequestMatchmaking()
{
	if (!IsLoggedIn()) return;

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("token"), AuthToken);

	FString Body;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	TCPClient->SendPacket(EDNet::C2S_MATCH_QUEUE_REQ, Body);
}

void UEDMatchmakingSubsystem::CancelMatchmaking()
{
	if (!IsLoggedIn()) return;

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("token"), AuthToken);

	FString Body;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	TCPClient->SendPacket(EDNet::C2S_MATCH_CANCEL, Body);
}

// ============================================================
//  Lobby
// ============================================================

void UEDMatchmakingSubsystem::LobbySetReady()
{
	if (!IsLoggedIn() || CurrentMatchId.IsEmpty()) return;

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("token"), AuthToken);
	Json->SetStringField(TEXT("match_id"), CurrentMatchId);

	FString Body;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	TCPClient->SendPacket(EDNet::C2S_LOBBY_READY, Body);
}

void UEDMatchmakingSubsystem::LobbyChangeTeam(int32 NewTeamId)
{
	if (!IsLoggedIn() || CurrentMatchId.IsEmpty()) return;

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("token"), AuthToken);
	Json->SetStringField(TEXT("match_id"), CurrentMatchId);
	Json->SetNumberField(TEXT("team"), NewTeamId);

	FString Body;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	TCPClient->SendPacket(EDNet::C2S_LOBBY_TEAM_CHANGE, Body);
}

// ============================================================
//  Packet Dispatch
// ============================================================

void UEDMatchmakingSubsystem::HandlePacket(uint16 OpCode, const FString& JsonBody)
{
	switch (OpCode)
	{
	case EDNet::S2C_LOGIN_RES:      HandleLoginRes(JsonBody);      break;
	case EDNet::S2C_MATCH_QUEUE_RES: HandleMatchQueueRes(JsonBody); break;
	case EDNet::S2C_MATCH_FOUND:    HandleMatchFound(JsonBody);    break;
	case EDNet::S2C_LOBBY_STATE:    HandleLobbyState(JsonBody);    break;
	case EDNet::S2C_GAME_START:     HandleGameStart(JsonBody);     break;
	default:
		UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Unknown OpCode: 0x%04X"), OpCode);
		break;
	}
}

void UEDMatchmakingSubsystem::HandleDisconnect()
{
	AuthToken.Empty();
	Nickname.Empty();
	CurrentMatchId.Empty();
	OnMatchServerDisconnected.Broadcast();
	UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Disconnected from MatchServer"));
}

// ============================================================
//  Individual Handlers
// ============================================================

void UEDMatchmakingSubsystem::HandleLoginRes(const FString& JsonBody)
{
	TSharedPtr<FJsonObject> Json;
	auto Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		OnLoginResult.Broadcast(false, TEXT(""), TEXT("parse_error"));
		return;
	}

	FString Result = Json->GetStringField(TEXT("result"));

	if (Result == TEXT("ok"))
	{
		AuthToken = Json->GetStringField(TEXT("token"));
		Nickname = Json->GetStringField(TEXT("nickname"));

		// Update GameInstance
		if (UEDGameInstance* GI = Cast<UEDGameInstance>(GetGameInstance()))
		{
			GI->LocalPlayerNickname = Nickname;
		}

		UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Login OK: %s"), *Nickname);
		OnLoginResult.Broadcast(true, Nickname, TEXT(""));
	}
	else
	{
		FString Reason = Json->GetStringField(TEXT("reason"));
		UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Login failed: %s"), *Reason);
		OnLoginResult.Broadcast(false, TEXT(""), Reason);
	}
}

void UEDMatchmakingSubsystem::HandleMatchQueueRes(const FString& JsonBody)
{
	UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Queued for matchmaking"));
	OnMatchQueued.Broadcast();
}

void UEDMatchmakingSubsystem::HandleMatchFound(const FString& JsonBody)
{
	TSharedPtr<FJsonObject> Json;
	auto Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid()) return;

	CurrentMatchId = Json->GetStringField(TEXT("match_id"));
	UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Match found: %s"), *CurrentMatchId);
	OnMatchFound.Broadcast(CurrentMatchId);
}

void UEDMatchmakingSubsystem::HandleLobbyState(const FString& JsonBody)
{
	OnLobbyStateChanged.Broadcast(JsonBody);
}

void UEDMatchmakingSubsystem::HandleGameStart(const FString& JsonBody)
{
	TSharedPtr<FJsonObject> Json;
	auto Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid()) return;

	FString ServerIP = Json->GetStringField(TEXT("server_ip"));
	int32 ServerPort = Json->GetIntegerField(TEXT("server_port"));

	UE_LOG(LogEDCore, Warning, TEXT("[MatchmakingSubsystem] Game start: %s:%d"), *ServerIP, ServerPort);

	OnGameStart.Broadcast(ServerIP, ServerPort);

	// Auto-join the dedicated server
	if (UEDGameInstance* GI = Cast<UEDGameInstance>(GetGameInstance()))
	{
		FString TravelURL = FString::Printf(TEXT("%s:%d"), *ServerIP, ServerPort);
		GI->JoinGame(TravelURL);
	}
}
