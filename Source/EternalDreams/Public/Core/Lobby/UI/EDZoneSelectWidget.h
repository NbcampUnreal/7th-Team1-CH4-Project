// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDZoneSelectWidget.generated.h"

class UButton;

/**
 * UEDZoneSelectWidget
 *
 * 로비에서 스폰 구역(1~4)을 선택하는 위젯.
 *
 * 사용법 (BP):
 *   1. 이 위젯을 상속하는 WBP 생성
 *   2. ZoneButton1 ~ ZoneButton4 이름으로 버튼 4개 배치 (BindWidget으로 자동 연결)
 *   3. OnZoneSelected 이벤트에서 선택된 구역 하이라이트 등 UI 갱신
 */
UCLASS()
class ETERNALDREAMS_API UEDZoneSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 구역 선택. 서버에 RPC로 전달한다.
	 * @param ZoneId 선택할 구역 번호 (1~4)
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|Lobby")
	void SelectZone(int32 ZoneId);

	/** 현재 선택된 구역 ID 반환 (0 = 미선택) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ED|Lobby")
	int32 GetSelectedZoneId() const;

	/** 구역 선택 시 BP에서 UI 갱신용으로 사용 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ED|Lobby")
	void OnZoneSelected(int32 ZoneId);

protected:
	virtual void NativeConstruct() override;

	// -------------------------------------------------------
	// BindWidget — BP에서 같은 이름의 버튼을 배치하면 자동 연결
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton4;

private:
	UFUNCTION()
	void OnZoneButton1Clicked();

	UFUNCTION()
	void OnZoneButton2Clicked();

	UFUNCTION()
	void OnZoneButton3Clicked();

	UFUNCTION()
	void OnZoneButton4Clicked();
};
