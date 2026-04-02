#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "EDUIManageSubsystem.generated.h"

class UEDHUDLayout;

UCLASS()
class ETERNALDREAMS_API UEDUIManageSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 에디터에서 만든 HUD 블루프린트 클래스를 외부에서 주입할 때 사용
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetHUDLayoutClass(TSubclassOf<UEDHUDLayout> InHUDLayoutClass);

	// HUD가 아직 없으면 생성하고 화면에 붙임 
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CreateHUD();

	// HUD가 없으면 먼저 생성한 뒤 표시 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHUD();

	// 현재 생성된 HUD를 숨김 상태로 전환
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideHUD();

	// 현재 생성되어 있는 HUD 인스턴스를 반환
	UFUNCTION(BlueprintPure, Category = "UI")
	UEDHUDLayout* GetHUDLayout() const;

protected:
	// 실제로 생성된 HUD 위젯 인스턴스를 보관
	UPROPERTY(Transient)
	TObjectPtr<UEDHUDLayout> HUDLayoutInstance;

	// 생성에 사용할 HUD 위젯 클래스를 보관
	UPROPERTY(Transient)
	TSubclassOf<UEDHUDLayout> HUDLayoutClass;

private:
	// HUD 인스턴스가 이미 생성되어 있는지 확인
	bool IsHUDCreated() const;

	// 내부 생성 로직을 수행하고, 성공 시 HUD 인스턴스 반환
	UEDHUDLayout* CreateHUDInternal();

	// 서브시스템 종료 또는 재정리 시 HUD를 안전하게 제거
	void CleanupHUD();
};
