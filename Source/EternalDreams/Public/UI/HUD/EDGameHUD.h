#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EDGameHUD.generated.h"

class UEDInventoryPanelWidget;
class UEDHUDLayout;
class UCommonActivatableWidget;
class UEDPauseMenuWidget;

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
	
	// 인벤토리 패널 BP 지정용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEDInventoryPanelWidget> InventoryPanelClass;
	
	// 일시정지 메뉴 지정용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEDPauseMenuWidget> PauseMenuPanelClass;

private:
	// 로컬 플레이어 UI 초기화 요청용
	void InitializeHUD() const;
	
	// 인벤토리 패널 열기/닫기
	void RunInventoryPanelOpenCloseTest() const;
};
