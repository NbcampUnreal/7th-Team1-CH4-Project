// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Network/EDDediServerSubsystem.h"
#include "Core/Network/EDTCPClient.h"
#include "Core/Network/EDNetProtocol.h"
#include "EternalDreams.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ============================================================
//  Lifecycle
// ============================================================

UEDDediServerSubsystem::UEDDediServerSubsystem()
	: TCPClient(nullptr)
	, ServerStatus(TEXT("idle"))
{
}

UEDDediServerSubsystem::~UEDDediServerSubsystem()
{
	if (TCPClient)
	{
		TCPClient->Disconnect();
		delete TCPClient;
		TCPClient = nullptr;
	}
}

bool UEDDediServerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only create on dedicated servers
	return IsRunningDedicatedServer();
}

void UEDDediServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Initialized (Dedicated Server)"));
}

void UEDDediServerSubsystem::Deinitialize()
{
	if (TCPClient)
	{
		TCPClient->Disconnect();
		delete TCPClient;
		TCPClient = nullptr;
	}

	// Clear heartbeat timer
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	}

	Super::Deinitialize();
}

// ============================================================
//  Connection
// ============================================================

bool UEDDediServerSubsystem::ConnectToMatchServer(const FString& Host, int32 Port)
{
	if (TCPClient && TCPClient->IsConnected())
	{
		UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Already connected to MatchServer"));
		return true;
	}

	TCPClient = new FEDTCPClient();

	TCPClient->OnPacketReceived.BindUObject(this, &UEDDediServerSubsystem::HandlePacket);
	TCPClient->OnDisconnected.BindUObject(this, &UEDDediServerSubsystem::HandleDisconnect);

	if (!TCPClient->Connect(Host, Port))
	{
		UE_LOG(LogEDCore, Error, TEXT("[DediServerSubsystem] Connect to MatchServer failed"));
		delete TCPClient;
		TCPClient = nullptr;
		return false;
	}

	UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Connected to MatchServer %s:%d"), *Host, Port);
	return true;
}

void UEDDediServerSubsystem::RegisterSelf(const FString& IP, int32 GamePort, int32 MaxPlayers)
{
	if (!TCPClient || !TCPClient->IsConnected())
	{
		UE_LOG(LogEDCore, Error, TEXT("[DediServerSubsystem] Not connected to MatchServer"));
		return;
	}

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("ip"), IP);
	Json->SetNumberField(TEXT("port"), GamePort);
	Json->SetNumberField(TEXT("max_players"), MaxPlayers);

	FString Body;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	TCPClient->SendPacket(EDNet::D2S_DEDI_REGISTER, Body);

	UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Registered: %s:%d (max %d)"), *IP, GamePort, MaxPlayers);

	// Start heartbeat timer (every 10 seconds)
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			HeartbeatTimerHandle,
			FTimerDelegate::CreateUObject(this, &UEDDediServerSubsystem::SendHeartbeat),
			10.0f, true);
	}
}

// ============================================================
//  Heartbeat
// ============================================================

void UEDDediServerSubsystem::SendHeartbeat()
{
	if (!TCPClient || !TCPClient->IsConnected()) return;

	int32 PlayerCount = 0;
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		PlayerCount = World->GetNumPlayerControllers();
	}

	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("server_id"), TEXT("self"));
	Json->SetStringField(TEXT("status"), ServerStatus);
	Json->SetNumberField(TEXT("player_count"), PlayerCount);

	FString Body;
	auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Body);
	FJsonSerializer::Serialize(Json.ToSharedRef(), Writer);

	TCPClient->SendPacket(EDNet::D2S_DEDI_HEARTBEAT, Body);
}

// ============================================================
//  Token verification (called from GameMode::PreLogin)
// ============================================================

bool UEDDediServerSubsystem::IsTokenAuthorized(const FString& Token) const
{
	return AuthorizedTokens.Contains(Token);
}

// ============================================================
//  Match result reporting
// ============================================================

void UEDDediServerSubsystem::ReportMatchResult(const FString& ResultJson)
{
	if (!TCPClient || !TCPClient->IsConnected()) return;

	TCPClient->SendPacket(EDNet::D2S_DEDI_MATCH_RESULT, ResultJson);

	// Reset state
	CurrentMatchId.Empty();
	AuthorizedTokens.Empty();
	ServerStatus = TEXT("idle");

	UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Match result reported, back to idle"));
}

// ============================================================
//  Packet Dispatch
// ============================================================

void UEDDediServerSubsystem::HandlePacket(uint16 OpCode, const FString& JsonBody)
{
	switch (OpCode)
	{
	case EDNet::S2D_DEDI_ASSIGN:
		HandleDediAssignMatch(JsonBody);
		break;
	default:
		UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Unknown OpCode: 0x%04X"), OpCode);
		break;
	}
}

void UEDDediServerSubsystem::HandleDisconnect()
{
	UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Disconnected from MatchServer"));
}

// ============================================================
//  DEDI_ASSIGN_MATCH handler
// ============================================================

void UEDDediServerSubsystem::HandleDediAssignMatch(const FString& JsonBody)
{
	UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] DEDI_ASSIGN_MATCH: %s"), *JsonBody);

	TSharedPtr<FJsonObject> Json;
	auto Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid()) return;

	CurrentMatchId = Json->GetStringField(TEXT("match_id"));

	// Store authorized tokens
	AuthorizedTokens.Empty();
	const TArray<TSharedPtr<FJsonValue>>* TokenArray;
	if (Json->TryGetArrayField(TEXT("auth_tokens"), TokenArray))
	{
		for (auto& TokenVal : *TokenArray)
		{
			AuthorizedTokens.Add(TokenVal->AsString());
		}
	}

	ServerStatus = TEXT("ingame");

	UE_LOG(LogEDCore, Warning, TEXT("[DediServerSubsystem] Match assigned: %s, %d authorized tokens"),
		*CurrentMatchId, AuthorizedTokens.Num());
}
