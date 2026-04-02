#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EDGameHUD.generated.h"

class UEDHUDLayout;
class UCommonActivatableWidget;

UCLASS()
class ETERNALDREAMS_API AEDGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	// 에디터에서 HUD 루트 BP 지정용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEDHUDLayout> HUDLayoutClass;
	
	// 테스트 패널 BP 지정용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCommonActivatableWidget> TestPanelClass;

private:
	// 로컬 플레이어 UI 초기화 요청용
	void InitializeHUD() const;
	
	// 패널 Open/Close 테스트
	void RunPanelOpenCloseTest() const;
};
