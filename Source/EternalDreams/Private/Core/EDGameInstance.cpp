// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameInstance.h"
#include "EternalDreams.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"
#include "MoviePlayer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

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

	// ============================================================
	// [비동기로드] 맵 프리로드 위치
	// ============================================================
	// 향후 IOCP 매치 성립(HandleMatchFound) 시점에
	// LoadPackageAsync(GameMapPath) 호출로 미리 맵을 캐싱하면,
	// 여기서의 ClientTravel이 더 빠르게 완료된다.
	// ============================================================

	// 로딩 화면 표시 → ClientTravel → 맵 로드 동안 유지
	ShowLoadingScreen();

	const FString TravelURL = ServerIP.Contains(TEXT(":")) ? ServerIP : ServerIP + TEXT(":7777");

	UE_LOG(LogEDCore, Warning, TEXT("[GameInstance] ClientTravel 시작 — URL: %s"), *TravelURL);
	PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

// ============================================================
//  로딩 화면
// ============================================================

void UEDGameInstance::ShowLoadingScreen()
{
	if (!IsRunningDedicatedServer())
	{
		FLoadingScreenAttributes LoadingScreen;

		// true: 맵 로드 완료 시 자동으로 로딩 화면 제거 (테스트 용이)
		// [IOCP 전환 시] false로 변경하고, 전원 접속 + 데이터 로드 완료 시 HideLoadingScreen() 호출
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
		LoadingScreen.bMoviesAreSkippable = false;
		LoadingScreen.MinimumLoadingScreenDisplayTime = 1.0f;

		// 기본 로딩 위젯 — 향후 UMG 위젯이나 커스텀 SWidget으로 교체 가능
		LoadingScreen.WidgetLoadingScreen =
			SNew(SBorder)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.BorderBackgroundColor(FLinearColor::Black)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Loading...")))
				.Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 40))
				.ColorAndOpacity(FLinearColor::White)
			];

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);

		UE_LOG(LogEDCore, Warning, TEXT("[GameInstance] 로딩 화면 시작"));
	}
}

void UEDGameInstance::HideLoadingScreen()
{
	if (!IsRunningDedicatedServer())
	{
		GetMoviePlayer()->StopMovie();
		UE_LOG(LogEDCore, Warning, TEXT("[GameInstance] 로딩 화면 종료"));
	}
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
