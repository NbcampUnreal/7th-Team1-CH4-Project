#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EDGameHUD.generated.h"

class UEDInventoryPanelWidget;
class UEDHUDLayout;
class UCommonActivatableWidget;
class UEDPauseMenuWidget;
class UEDUIRegistryDataAsset;
class UEDUIManageSubsystem;

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
	
	// 에디터에서 어떤 UI를 등록할지 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UEDUIRegistryDataAsset> UIRegistry;
	
	// Registry에 등록된 UI를 Subsystem에 반영
	void RegisterWidgetsFromRegistry(UEDUIManageSubsystem* UIManageSubsystem) const;

private:
	// 로컬 플레이어 UI 초기화 요청용
	void InitializeHUD() const;
};
