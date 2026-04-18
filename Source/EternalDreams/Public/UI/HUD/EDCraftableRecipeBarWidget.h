#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDCraftableRecipeBarWidget.generated.h"

class AEDPlayerController;
class UPanelWidget;
class UEDCraftableRecipeSlotWidget;
class UEDInventoryComponent;
class UTexture2D;

/**
 * 현재 제작 가능한 아이템들을 아이콘 가로 바 형태로 보여주는 HUD 위젯
 * 첫 번째 슬롯은 지금 제작 핫키로 만들어질 아이템을 뜻함.
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftableRecipeBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 외부에서 인벤토리 컴포넌트를 직접 지정
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void SetInventoryComponent(UEDInventoryComponent* InInventoryComponent);

	// 현재 제작 가능 아이템 바를 즉시 다시 구성
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void RefreshCraftableRecipeBar();

protected:
	// 제작 가능 아이템 슬롯들이 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> CraftableRecipeBarContainer;

	// 생성할 제작 가능 아이템 슬롯 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftableRecipeSlotWidget> CraftableRecipeSlotWidgetClass;

private:
	// 소유 플레이어에서 인벤토리 컴포넌트를 찾기
	void InitializeInventoryComponent();

	// 인벤토리 변경 이벤트 바인딩
	void BindInventoryChanged();

	// 인벤토리 변경 이벤트 바인딩 해제
	void UnbindInventoryChanged();

	// 플레이어 컨트롤러의 제작 핫키 입력 델리게이트 바인딩
	void BindCraftInputTriggered();

	// 플레이어 컨트롤러의 제작 핫키 입력 델리게이트 바인딩 해제
	void UnbindCraftInputTriggered();

	// 제작 가능 레시피 하나의 표시 정보를 계산
	bool ResolveRecipeDisplayData(const FEDCraftableRecipeEntry& InRecipeData, UTexture2D*& OutIconTexture) const;

	// 인벤토리 변경 시 바를 갱신
	UFUNCTION()
	void HandleInventoryChanged();

	// 제작 핫키 입력 시 바를 갱신
	void HandleCraftInputTriggered();

private:
	// 현재 바가 참조하는 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 현재 화면에 생성된 슬롯 위젯들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UEDCraftableRecipeSlotWidget>> CraftableRecipeSlotWidgets;

	// 제작 핫키 입력 델리게이트를 구독 중인 플레이어 컨트롤러
	UPROPERTY(Transient)
	TObjectPtr<AEDPlayerController> BoundPlayerController;
};
