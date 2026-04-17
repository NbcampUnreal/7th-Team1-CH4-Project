// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "EDGameInstance.generated.h"

class UNetDriver;

/**
 * UEDGameInstance
 *
 * 앱 수명 동안 유지되는 싱글턴 오브젝트.
 * 데디케이티드 서버 방식 — 클라이언트는 서버 IP로 직접 접속.
 * 레벨 전환 간 유지 데이터를 보관한다.
 */
UCLASS()
class ETERNALDREAMS_API UEDGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UEDGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	// -------------------------------------------------------
	// 레벨 전환 간 유지 데이터
	// -------------------------------------------------------

	/** 로컬 플레이어 닉네임 */
	UPROPERTY(BlueprintReadWrite, Category = "ED|GameInstance")
	FString LocalPlayerNickname;

	/** 희망 팀 ID (EDTeam::None = 미배정) */
	UPROPERTY(BlueprintReadWrite, Category = "ED|GameInstance")
	int32 DesiredTeamID = -1;  // EDTeam::None

	/** 마지막으로 접속을 시도한 서버 IP */
	UPROPERTY(BlueprintReadWrite, Category = "ED|GameInstance")
	FString LastServerIP;

	// -------------------------------------------------------
	// 레벨 트래블
	// -------------------------------------------------------

	/** 데디케이티드 서버 IP로 접속. 로컬 테스트: "127.0.0.1" */
	UFUNCTION(BlueprintCallable, Category = "ED|GameInstance")
	void JoinGame(const FString& ServerIP);

	// -------------------------------------------------------
	// 로딩 화면
	// -------------------------------------------------------

	/** 로딩 화면을 표시한다. JoinGame 내부에서 자동 호출됨 */
	UFUNCTION(BlueprintCallable, Category = "ED|GameInstance")
	void ShowLoadingScreen();

	/** 로딩 화면을 수동으로 제거한다. (bAutoComplete=false일 때 사용) */
	UFUNCTION(BlueprintCallable, Category = "ED|GameInstance")
	void HideLoadingScreen();

	// -------------------------------------------------------
	// 로딩 화면 — 배경 이미지 (에디터에서 지정)
	// -------------------------------------------------------

	/** 로딩 화면 배경 이미지. GameInstance BP 디테일에서 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "ED|Loading")
	TSoftObjectPtr<UTexture2D> LoadingBackgroundImage;

private:
	/** GC 방지용 텍스처 강한 참조 */
	UPROPERTY()
	TObjectPtr<UTexture2D> LoadingBackgroundTexture;

	/** Init에서 미리 로드한 배경 브러시 */
	FSlateBrush LoadingBackgroundBrush;

	/** 맵 로드 시작 시 엔진이 호출하는 콜백 — 자동으로 로딩 화면 표시 */
	void OnPreLoadMap(const FString& MapName);

	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);
};
