// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameInstance.h"
#include "EternalDreams.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"

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
	UE_LOG(LogEDCore, Warning, TEXT("[GameInstance] JoinGame 호출 — ServerIP: %s"), *ServerIP);

	LastServerIP = ServerIP;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		UE_LOG(LogEDCore, Error, TEXT("[GameInstance] JoinGame 실패 — PlayerController 없음"));
		return;
	}

	const FString TravelURL = ServerIP.Contains(TEXT(":")) ? ServerIP : ServerIP + TEXT(":7777");

	UE_LOG(LogEDCore, Warning, TEXT("[GameInstance] ClientTravel 시작 — URL: %s"), *TravelURL);
	PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

void UEDGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogEDCore, Error, TEXT("[GameInstance] NetworkFailure — Type: %d, Error: %s, World: %s"),
		static_cast<int32>(FailureType), *ErrorString, World ? *World->GetName() : TEXT("null"));
}

void UEDGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogEDCore, Error, TEXT("[GameInstance] TravelFailure — Type: %d, Error: %s, World: %s"),
		static_cast<int32>(FailureType), *ErrorString, World ? *World->GetName() : TEXT("null"));
}
