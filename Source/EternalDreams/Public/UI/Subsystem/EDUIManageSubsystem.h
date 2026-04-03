#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI/Types/EDUITypes.h"
#include "EDUIManageSubsystem.generated.h"

class UEDHUDLayout;
class UCommonActivatableWidget;

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

	// 패널 클래스 등록
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RegisterPanelClass(FName PanelId, EEDUILayer Layer, TSubclassOf<UCommonActivatableWidget> PanelClass);

	// 패널 열기
	UFUNCTION(BlueprintCallable, Category = "UI")
	UCommonActivatableWidget* OpenPanel(FName PanelId);

	// 패널 닫기
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ClosePanel(FName PanelId);

	// 패널 토글
	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePanel(FName PanelId);

	// 패널 열림 여부 확인
	UFUNCTION(BlueprintCallable, Category = "UI")
	bool IsPanelOpen(FName PanelId) const;

protected:
	// 실제로 생성된 HUD 위젯 인스턴스를 보관
	UPROPERTY(Transient)
	TObjectPtr<UEDHUDLayout> HUDLayoutInstance;

	// 생성에 사용할 HUD 위젯 클래스를 보관
	UPROPERTY(Transient)
	TSubclassOf<UEDHUDLayout> HUDLayoutClass;

	// 패널 ID별 클래스 보관
	UPROPERTY(Transient)
	TMap<FName, TSubclassOf<UCommonActivatableWidget>> RegisteredPanelClasses;

	// 생성된 패널 인스턴스 보관
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UCommonActivatableWidget>> PanelInstances;

	// 패널 ID별 레이어 정보 보관
	UPROPERTY(Transient)
	TMap<FName, EEDUILayer> RegisteredPanelLayers;

private:
	// HUD 인스턴스가 이미 생성되어 있는지 확인
	bool IsHUDCreated() const;

	// 내부 생성 로직을 수행하고, 성공 시 HUD 인스턴스 반환
	UEDHUDLayout* CreateHUDInternal();

	// 서브시스템 종료 또는 재정리 시 HUD를 안전하게 제거
	void CleanupHUD();

	// 패널 없으면 생성
	UCommonActivatableWidget* CreatePanelInstance(FName PanelId);
	
	// 패널 레이어 조회
	EEDUILayer GetPanelLayer(FName PanelId) const;
	
	// 패널을 레이어 슬롯에 부착
	bool AttachPanelToLayer(FName PanelId, UCommonActivatableWidget* PanelInstance);
};
