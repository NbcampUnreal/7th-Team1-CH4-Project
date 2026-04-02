// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EDGameInstance.generated.h"

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

	/** 희망 팀 ID (-1 = 미배정) */
	UPROPERTY(BlueprintReadWrite, Category = "ED|GameInstance")
	int32 DesiredTeamID = -1;

	/** 마지막으로 접속을 시도한 서버 IP */
	UPROPERTY(BlueprintReadWrite, Category = "ED|GameInstance")
	FString LastServerIP;

	// -------------------------------------------------------
	// 레벨 트래블
	// -------------------------------------------------------

	/** 데디케이티드 서버 IP로 접속. 로컬 테스트: "127.0.0.1" */
	UFUNCTION(BlueprintCallable, Category = "ED|GameInstance")
	void JoinGame(const FString& ServerIP);


};
