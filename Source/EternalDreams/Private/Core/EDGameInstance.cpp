// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const TCHAR* NetModeToString(const UWorld* World)
	{
		if (!World)
		{
			return TEXT("NoWorld");
		}

		switch (World->GetNetMode())
		{
		case NM_Standalone:
			return TEXT("Standalone");
		case NM_DedicatedServer:
			return TEXT("DedicatedServer");
		case NM_ListenServer:
			return TEXT("ListenServer");
		case NM_Client:
			return TEXT("Client");
		default:
			return TEXT("Unknown");
		}
	}

}

UEDGameInstance::UEDGameInstance()
{
}

void UEDGameInstance::Init()
{
	Super::Init();

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UEDGameInstance::HandleNetworkFailure);
		GEngine->OnTravelFailure().AddUObject(this, &UEDGameInstance::HandleTravelFailure);
	}
}

void UEDGameInstance::Shutdown()
{
	if (GEngine)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
		GEngine->OnTravelFailure().RemoveAll(this);
	}

	Super::Shutdown();
}

void UEDGameInstance::JoinGame(const FString& ServerIP)
{
	LastServerIP = ServerIP;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameInstance] JoinGame failed - FirstLocalPlayerController is null"));
		return;
	}

	const FString TravelURL = ServerIP.Contains(TEXT(":")) ? ServerIP : ServerIP + TEXT(":7777");
	UWorld* World = PC->GetWorld();

	UE_LOG(LogTemp, Log, TEXT("[GameInstance] JoinGame requested | InputIP=%s | TravelURL=%s | Map=%s | NetMode=%s | HasAuthority=%s"),
		*ServerIP,
		*TravelURL,
		World ? *World->GetMapName() : TEXT("None"),
		NetModeToString(World),
		PC->HasAuthority() ? TEXT("true") : TEXT("false"));

	PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

void UEDGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("[GameInstance] NetworkFailure | Type=%d | Error=%s | NetDriver=%s | Map=%s | NetMode=%s"),
		static_cast<int32>(FailureType),
		*ErrorString,
		NetDriver ? *NetDriver->GetName() : TEXT("None"),
		World ? *World->GetMapName() : TEXT("None"),
		NetModeToString(World));
}

void UEDGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("[GameInstance] TravelFailure | Type=%d | Error=%s | Map=%s | NetMode=%s"),
		static_cast<int32>(FailureType),
		*ErrorString,
		World ? *World->GetMapName() : TEXT("None"),
		NetModeToString(World));
}
