#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "Engine/StreamableManager.h"
#include "EDInventoryQuickBarWidget.generated.h"

enum class EEDInventoryActionFailure : uint8;
struct FEDInventorySlotData;
class UUniformGridPanel;
class UEDInventoryComponent;
class UEDEquipmentSlotWidget;
class UTextBlock;
class UEDQuickBarSlotWidget;
class UEDInventoryQuantityPopupWidget;
class UEDInventoryItemDataAsset;

UCLASS()
class ETERNALDREAMS_API UEDInventoryQuickBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// 하단 퀵 슬롯이 배치될 그리드
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UUniformGridPanel> QuickSlotGrid;

	// 무기 슬롯 UI
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDEquipmentSlotWidget> WeaponSlotWidget;

	// 상의 슬롯 UI
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDEquipmentSlotWidget> TopArmorSlotWidget;

	// 하의 슬롯 UI
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDEquipmentSlotWidget> BottomArmorSlotWidget;

	// 한 줄당 슬롯 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 QuickSlotsPerRow = 5;

	// 표시할 퀵 슬롯 수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 QuickSlotCount = 10;
	
	// 생성할 퀵바 슬롯 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UEDQuickBarSlotWidget> QuickSlotWidgetClass;

	// 생성된 퀵바 슬롯 위젯 배열
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UEDQuickBarSlotWidget>> QuickSlotWidgets;

	// 현재 참조 중인 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;
	
	// 퀵바 액션 실패/안내 메시지 표시용 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> ActionResultText;

	// 일부 버리기 수량 선택 팝업
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryQuantityPopupWidget> QuantityPopupWidget;

private:
	void InitializeInventoryComponent();
	void BindInventoryChanged();
	void UnbindInventoryChanged();

	void CreateQuickSlotWidgets();
	void RefreshQuickSlots();
	void RefreshEquipmentSlots();

	UFUNCTION()
	void HandleInventoryChanged();

	FText ResolveItemDisplayName(const FPrimaryAssetId& ItemId) const;
	EEDItemRarity ResolveItemRarity(const FPrimaryAssetId& ItemId) const;
	
	// 현재 선택된 퀵바 슬롯 인덱스
	int32 SelectedSlotIndex = INDEX_NONE;

	// 퀵바 슬롯 좌클릭 처리
	void HandleQuickSlotClicked(int32 InSlotIndex);

	// 퀵바 슬롯 더블 클릭 처리
	void HandleQuickSlotDoubleClicked(int32 InSlotIndex);

	// 슬롯 선택 상태 갱신
	void RefreshSelectedSlotState();

	// 퀵바 인덱스로 슬롯 데이터를 얻음
	bool TryGetQuickSlotData(int32 QuickIndex, FEDInventorySlotData& OutSlotData) const;

	// 인벤토리 액션 실패 메시지 표시
	void ShowInventoryFailure(EEDInventoryActionFailure Failure) const;

	// 인벤토리 액션 메시지 지우기
	void ClearInventoryActionMessage() const;
	
	// 슬롯 간 드롭 처리
	void HandleQuickSlotDroppedOnSlot(int32 FromSlotIndex, int32 ToSlotIndex);

	// 퀵바 밖으로 드롭 처리
	void HandleQuickSlotDroppedOutside(int32 FromSlotIndex);

	// 슬롯 간 이동 시도
	void TryMoveQuickSlotItem(int32 FromSlotIndex, int32 ToSlotIndex);

	// 슬롯 일부 버리기 시도
	void TryDropQuickSlotItemPartial(int32 FromSlotIndex, int32 DropQuantity);

	// 일부 버리기 팝업 열기
	void OpenDropQuantityPopup(int32 FromSlotIndex, int32 MaxQuantity);

	// 일부 버리기 팝업 닫기
	void CloseDropQuantityPopup();

	// 수량 선택 팝업 확인 처리
	void HandleQuantityPopupConfirmed(int32 SelectedQuantity);

	// 수량 선택 팝업 취소 처리
	void HandleQuantityPopupCanceled();

	// 수량 팝업으로 넘길 대기 중 슬롯
	int32 PendingDropSlotIndex = INDEX_NONE;
	
	// 인벤토리 에셋을 비동기 로드 요청
	UFUNCTION()
	void PreloadInventoryAssets();
	// 비동기 요청에 대한 콜백
	void DoRefresh();
	// ResolveItemDisplayName + ResolveItemRarity를 합쳐서 비동기로 만든 함수
	const UEDInventoryItemDataAsset* ResolveItemData(const FPrimaryAssetId& ItemId) const;
	// 리프레시 방지 변수
	bool bRefreshPending = false;
	// GC방지용 핸들
	TSharedPtr<FStreamableHandle> PreloadHandle;
};
