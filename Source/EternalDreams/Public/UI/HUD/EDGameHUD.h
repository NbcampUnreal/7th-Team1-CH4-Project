#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EDGameHUD.generated.h"

class UEDHUDLayout;

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

private:
	// 로컬 플레이어 UI 초기화 요청용
	void InitializeHUD() const;
};
