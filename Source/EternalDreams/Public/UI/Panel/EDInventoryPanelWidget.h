#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Item/Core/EDItemTypes.h"
#include "EDInventoryPanelWidget.generated.h"

enum class EEDInventoryActionFailure : uint8;
struct FEDInventorySlotData;
class UUniformGridPanel;
class UTextBlock;
class UEDInventoryComponent;
class UEDInventorySlotWidget;

UCLASS()
class ETERNALDREAMS_API UEDInventoryPanelWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 패널이 표시할 외부 인벤토리 컴포넌트 설정
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetDisplayedInventoryComponent(UEDInventoryComponent* InInventoryComponent);

protected:
	// 인벤토리 슬롯이 배치될 그리드
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UUniformGridPanel> InventoryGrid;

	// 패널 제목 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> TitleText;

	// 현재 슬롯 사용량 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> CapacityText;

	// 컨테이너 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> ContainerNameText;

	// 생성할 슬롯 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UEDInventorySlotWidget> InventorySlotWidgetClass;

	// 한 줄당 슬롯 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 SlotsPerRow = 5;

	// 현재 패널이 표시 중인 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> DisplayedInventoryComponent;

	// 생성된 슬롯 위젯 배열
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory")
	TArray<TObjectPtr<UEDInventorySlotWidget>> InventorySlotWidgets;
	
	// 루팅 액션 결과 메시지 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> ActionResultText;

private:
	// 플레이어 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEDInventoryComponent> PlayerInventoryComponent;

	// 그리드에 슬롯 위젯 생성
	void CreateInventorySlotWidgets();

	// 현재 표시 대상 인벤토리를 기준으로 슬롯 UI 갱신
	void RefreshInventorySlots();

	// 슬롯 사용량 텍스트를 갱신
	void RefreshCapacityText() const;

	// 표시 대상 인벤토리 변경 델리게이트에 바인딩
	void BindInventoryChanged();

	// 표시 대상 인벤토리 변경 델리게이트 바인딩 해제
	void UnbindInventoryChanged();

	// 플레이어 인벤토리 컴포넌트 찾기
	void InitializePlayerInventoryComponent();

	// 외부 인벤토리 변경 시 호출
	UFUNCTION()
	void HandleInventoryChanged();

	// 루팅 슬롯 좌클릭 더블 클릭 처리
	void HandleLootSlotDoubleClicked(int32 InSlotIndex);

	// 외부 인벤토리 슬롯에서 플레이어 인벤토리로 아이템 이동
	void TryTransferItemToPlayerInventory(int32 InSlotIndex);

	// ItemId로 아이템 표시 이름 가져오기
	FText ResolveItemDisplayName(const FPrimaryAssetId& ItemId) const;

	// ItemId로 아이템 희귀도 가져오기
	EEDItemRarity ResolveItemRarity(const FPrimaryAssetId& ItemId) const;

	// 슬롯 인덱스로부터 데이터 접근 가능 여부 확인
	bool TryGetSlotData(int32 InSlotIndex, FEDInventorySlotData& OutSlotData) const;
	
	// 현재 선택된 루팅 슬롯 인덱스
	int32 SelectedSlotIndex = INDEX_NONE;
	
	// 루팅 슬롯 좌클릭 처리
	void HandleLootSlotClicked(int32 InSlotIndex);
	
	// 선택된 슬롯 시각 상태 갱신
	void RefreshSelectedSlotState();
	
	void ShowInventoryFailure(EEDInventoryActionFailure Failure) const;
	void ClearInventoryActionMessage() const;
};
