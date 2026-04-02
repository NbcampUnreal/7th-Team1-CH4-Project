// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameInstance.h"
#include "GameFramework/PlayerController.h"

UEDGameInstance::UEDGameInstance()
{
}

void UEDGameInstance::Init()
{
	Super::Init();
}

void UEDGameInstance::Shutdown()
{
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
