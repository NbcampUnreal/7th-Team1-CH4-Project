// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDZoneSelectWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * UEDZoneSelectWidget
 *
 * 로비에서 스폰 구역(A~D)을 선택하는 위젯.
 *
 * 사용법 (BP):
 *   1. 이 위젯을 상속하는 WBP 생성
 *   2. ZoneButton1~4, ZoneLabel 이름으로 위젯 배치 (BindWidget 자동 연결)
 *   3. 선택된 버튼은 자동으로 하이라이트됨
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

	/** 구역 선택 시 BP에서 추가 UI 갱신용으로 사용 */
	UFUNCTION(BlueprintImplementableEvent, Category = "ED|Lobby")
	void OnZoneSelected(int32 ZoneId);

protected:
	virtual void NativeConstruct() override;

	// -------------------------------------------------------
	// BindWidget — BP에서 같은 이름의 위젯을 배치하면 자동 연결
	// -------------------------------------------------------

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ZoneButton4;

	/** 현재 선택된 구역 표시 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ZoneLabel;

	// -------------------------------------------------------
	// 하이라이트 색상 (에디터에서 조정 가능)
	// -------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, Category = "ED|Lobby|Style")
	FLinearColor SelectedColor = FLinearColor(0.2f, 0.6f, 1.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "ED|Lobby|Style")
	FLinearColor NormalColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

private:
	void UpdateButtonVisuals(int32 SelectedZoneId);
	UButton* GetButtonByZoneId(int32 ZoneId) const;

	UFUNCTION() void OnZone1Clicked() { SelectZone(1); }
	UFUNCTION() void OnZone2Clicked() { SelectZone(2); }
	UFUNCTION() void OnZone3Clicked() { SelectZone(3); }
	UFUNCTION() void OnZone4Clicked() { SelectZone(4); }

	static const TCHAR* ZoneNames[4];
};
