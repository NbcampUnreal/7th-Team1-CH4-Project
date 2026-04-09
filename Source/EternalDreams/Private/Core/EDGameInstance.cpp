// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogEDNetwork, Log, All);

namespace
{
	template <typename TEnum>
	FString GetEnumValueString(const TCHAR* EnumPath, TEnum Value)
	{
		if (const UEnum* Enum = FindObject<UEnum>(nullptr, EnumPath))
		{
			return Enum->GetNameStringByValue(static_cast<int64>(Value));
		}

		return FString::Printf(TEXT("%d"), static_cast<int32>(Value));
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
	LastServerIP = ServerIP.TrimStartAndEnd();
	LastConnectionError.Reset();

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		LastConnectionError = TEXT("LocalPlayerController is not available.");
		UE_LOG(LogEDNetwork, Error, TEXT("JoinGame failed before ClientTravel. Reason=%s"), *LastConnectionError);
		return;
	}

	const FString TravelURL = LastServerIP.Contains(TEXT(":")) ? LastServerIP : LastServerIP + TEXT(":7777");
	if (TravelURL.IsEmpty())
	{
		LastConnectionError = TEXT("Server address is empty.");
		UE_LOG(LogEDNetwork, Error, TEXT("JoinGame failed before ClientTravel. Reason=%s"), *LastConnectionError);
		return;
	}

	UE_LOG(LogEDNetwork, Log, TEXT("JoinGame requested. Input='%s', TravelURL='%s'"), *LastServerIP, *TravelURL);
	PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

void UEDGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	const FString FailureTypeString = GetEnumValueString(TEXT("/Script/Engine.ENetworkFailure"), FailureType);
	const FString WorldName = World ? World->GetName() : TEXT("None");
	const FString NetDriverName = NetDriver ? NetDriver->GetName() : TEXT("None");

	LastConnectionError = FString::Printf(TEXT("NetworkFailure(%s): %s"), *FailureTypeString, *ErrorString);
	UE_LOG(LogEDNetwork, Error, TEXT("Network failure. Type=%s Error='%s' World=%s NetDriver=%s LastServerIP='%s'"),
		*FailureTypeString, *ErrorString, *WorldName, *NetDriverName, *LastServerIP);
}

void UEDGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	const FString FailureTypeString = GetEnumValueString(TEXT("/Script/Engine.ETravelFailure"), FailureType);
	const FString WorldName = World ? World->GetName() : TEXT("None");

	LastConnectionError = FString::Printf(TEXT("TravelFailure(%s): %s"), *FailureTypeString, *ErrorString);
	UE_LOG(LogEDNetwork, Error, TEXT("Travel failure. Type=%s Error='%s' World=%s LastServerIP='%s'"),
		*FailureTypeString, *ErrorString, *WorldName, *LastServerIP);
}
