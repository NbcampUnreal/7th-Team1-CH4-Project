// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameInstance.h"
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
	LastServerIP = ServerIP;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		return;
	}

	const FString TravelURL = ServerIP.Contains(TEXT(":")) ? ServerIP : ServerIP + TEXT(":7777");

	PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

void UEDGameInstance::HandleNetworkFailure(UWorld* /*World*/, UNetDriver* /*NetDriver*/, ENetworkFailure::Type /*FailureType*/, const FString& /*ErrorString*/)
{
}

void UEDGameInstance::HandleTravelFailure(UWorld* /*World*/, ETravelFailure::Type /*FailureType*/, const FString& /*ErrorString*/)
{
}
