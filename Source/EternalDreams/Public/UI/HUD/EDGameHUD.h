#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Engine/StreamableManager.h"
#include "EDGameHUD.generated.h"

class UEDUIRegistryDataAsset;
class UEDUIManageSubsystem;

UCLASS()
class ETERNALDREAMS_API AEDGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	// 에디터에서 어떤 UI를 등록할지 지정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UEDUIRegistryDataAsset> UIRegistry;

	// Registry에 등록된 UI를 Subsystem에 반영
	void RegisterWidgetsFromRegistry(UEDUIManageSubsystem* UIManageSubsystem);

private:
	// 로컬 플레이어 UI 초기화 요청용
	void InitializeHUD();
	
	// 비동기 로드 후 실제 등록
	void OnWidgetClassesLoaded(UEDUIManageSubsystem* UIManageSubsystem);
	// 핸들 보관
	TSharedPtr<FStreamableHandle> WidgetClassLoadHandle;
};
