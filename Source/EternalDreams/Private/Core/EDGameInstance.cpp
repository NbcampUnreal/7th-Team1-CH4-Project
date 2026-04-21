// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/EDGameInstance.h"
#include "EternalDreams.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"
#include "MoviePlayer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"

#include "Widgets/SNullWidget.h"

UEDGameInstance::UEDGameInstance()
{
}

void UEDGameInstance::SetLocalPlayerNickname(const FString& InNickname)
{
	LocalPlayerNickname = InNickname.TrimStartAndEnd();
}

FString UEDGameInstance::GetLocalPlayerNickname() const
{
	return LocalPlayerNickname;
}

void UEDGameInstance::Init()
{
	Super::Init();

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UEDGameInstance::HandleNetworkFailure);
		GEngine->OnTravelFailure().AddUObject(this, &UEDGameInstance::HandleTravelFailure);
	}

	// 로딩 배경 이미지를 앱 시작 시 미리 로드 (첫 실행 누락 방지)
	if (!LoadingBackgroundImage.IsNull())
	{
		LoadingBackgroundTexture = LoadingBackgroundImage.LoadSynchronous();
		if (LoadingBackgroundTexture)
		{
			LoadingBackgroundBrush.SetResourceObject(LoadingBackgroundTexture);
			LoadingBackgroundBrush.ImageSize = FVector2D(LoadingBackgroundTexture->GetSizeX(), LoadingBackgroundTexture->GetSizeY());
			LoadingBackgroundBrush.DrawAs = ESlateBrushDrawType::Image;
			LoadingBackgroundBrush.Tiling = ESlateBrushTileType::NoTile;

		}
	}

	// 모든 맵 로드 시 자동으로 로딩 화면 표시 (ClientTravel, ServerTravel 모두 대응)
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEDGameInstance::OnPreLoadMap);
}

void UEDGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);

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

	// 로딩 화면은 PreLoadMap 콜백에서 자동 표시됨

	const FString TravelURL = ServerIP.Contains(TEXT(":")) ? ServerIP : ServerIP + TEXT(":7777");

	UE_LOG(LogEDCore, Warning, TEXT("[GameInstance] ClientTravel 시작 — URL: %s"), *TravelURL);
	PC->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
}

// ============================================================
//  로딩 화면
// ============================================================

void UEDGameInstance::OnPreLoadMap(const FString& MapName)
{
	ShowLoadingScreen();
}

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

		// 배경: 이미지가 지정되어 있으면 이미지, 없으면 단색
		const bool bHasBackground = LoadingBackgroundBrush.GetResourceObject() != nullptr;

		LoadingScreen.WidgetLoadingScreen =
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.05f, 1.0f))
			[
				SNew(SVerticalBox)

				// 중앙: 배경 이미지 (100%)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					bHasBackground
					? SNew(SScaleBox)
						.Stretch(EStretch::ScaleToFit)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(&LoadingBackgroundBrush)
						]
					: SNullWidget::NullWidget
				]

				// 하단: 스피너
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 80.0f)
				[
					SNew(SCircularThrobber)
					.Radius(24.0f)
					.NumPieces(8)
				]
			];

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
}

void UEDGameInstance::HideLoadingScreen()
{
	if (!IsRunningDedicatedServer())
	{
		GetMoviePlayer()->StopMovie();
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
