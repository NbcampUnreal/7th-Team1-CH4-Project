#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventoryPanelWidget.generated.h"

class UUniformGridPanel;
class UTextBlock;
class UEDInventoryComponent;
class UEDInventorySlotWidget;
class UEDEquipmentSlotWidget;

UCLASS()
class ETERNALDREAMS_API UEDInventoryPanelWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 인벤토리 슬롯이 배치될 그리드
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UUniformGridPanel> InventoryGrid;

	// 패널 제목
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> TitleText;

	// 현재 슬롯 사용량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> CapacityText;

	// 하단 안내 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> HintText;

	// 생성할 슬롯 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UEDInventorySlotWidget> InventorySlotWidgetClass;

	// 한 줄당 슬롯 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 SlotsPerRow = 5;

	// 현재 패널이 참조하는 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 생성된 슬롯 위젯 배열
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UEDInventorySlotWidget>> InventorySlotWidgets;
	
	// 무기 슬롯 UI
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UEDEquipmentSlotWidget> WeaponSlotWidget;

	// 상의 슬롯 UI
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UEDEquipmentSlotWidget> TopArmorSlotWidget;

	// 하의 슬롯 UI
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Equipment")
	TObjectPtr<UEDEquipmentSlotWidget> BottomArmorSlotWidget;

private:
	// 플레이어에서 인벤토리 컴포넌트를 찾음
	void InitializeInventoryComponent();

	// 그리드에 슬롯 위젯을 생성
	void CreateInventorySlotWidgets();

	// 현재 인벤토리 데이터를 기준으로 슬롯 UI를 갱신
	void RefreshInventorySlots();

	// 슬롯 사용량 텍스트를 갱신
	void RefreshCapacityText() const;

	// 인벤토리 변경 델리게이트에 바인딩
	void BindInventoryChanged();

	// 인벤토리 변경 델리게이트 바인딩을 해제
	void UnbindInventoryChanged();

	// 인벤토리 변경 시 호출
	UFUNCTION()
	void HandleInventoryChanged();

	// ItemId로 아이템 표시 이름을 구함
	FText ResolveItemDisplayName(const FPrimaryAssetId& ItemId) const;

	// ItemId로 아이템 희귀도를 구함
	EEDItemRarity ResolveItemRarity(const FPrimaryAssetId& ItemId) const;
	
	// 장비 슬롯 UI를 갱신
	void RefreshEquipmentSlots();
};
