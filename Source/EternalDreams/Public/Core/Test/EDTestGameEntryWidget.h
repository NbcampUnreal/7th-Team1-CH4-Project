// Copyright Eternal Dreams Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDTestGameEntryWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;

/**
 * UEDTestGameEntryWidget
 *
 * 게임 입장 테스트용 UI.
 * 닉네임 입력 / 서버 IP 입력 / 접속 기능만 포함.
 *
 * 사용법:
 *   에디터에서 이 클래스를 부모로 하는 WBP_TestGameEntry 위젯 블루프린트를 생성.
 *   BindWidget 변수명과 동일한 이름으로 UMG 컴포넌트를 배치.
 */
UCLASS()
class ETERNALDREAMS_API UEDTestGameEntryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// -------------------------------------------------------
	// UMG 바인딩 (에디터에서 동일한 이름으로 배치)
	// -------------------------------------------------------

	/** 닉네임 입력 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> NicknameInput;

	/** 서버 IP 입력 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> ServerIPInput;

	/** 접속 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	/** 게임 시작 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	/** 상태 메시지 출력 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StatusText;

	// -------------------------------------------------------
	// 버튼 콜백
	// -------------------------------------------------------

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnStartClicked();

private:
	/** 닉네임을 GameInstance에 저장. 비어있으면 StatusText 출력 후 false 반환. */
	bool TrySaveNickname();
};
