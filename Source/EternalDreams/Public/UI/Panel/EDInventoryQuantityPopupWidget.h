#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "EDInventoryQuantityPopupWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEDInventoryQuantityConfirmed, int32);
DECLARE_MULTICAST_DELEGATE(FOnEDInventoryQuantityCanceled);

UCLASS()
class ETERNALDREAMS_API UEDInventoryQuantityPopupWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// 팝업에 사용할 최소/최대/초기 수량 설정
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetupQuantityRange(int32 InMinQuantity, int32 InMaxQuantity, int32 InInitialQuantity = 1);

	// 현재 선택 수량을 반환
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetCurrentQuantity() const;

	// 팝업을 기본 상태로 초기화하고 숨김
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ResetPopupState();

	// 확인 시 현재 선택 수량을 전달
	FOnEDInventoryQuantityConfirmed OnQuantityConfirmed;

	// 취소 시 호출
	FOnEDInventoryQuantityCanceled OnQuantityCanceled;

protected:
	// 팝업 제목 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> TitleText;

	// 현재 수량 표시 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UTextBlock> QuantityText;

	// 최소값 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UButton> MinusButton;

	// 최대값 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UButton> PlusButton;

	// 확인 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UButton> ConfirmButton;

	// 취소 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UButton> CancelButton;

private:
	// 현재 선택 수량
	int32 CurrentQuantity = 1;

	// 최소 수량
	int32 MinQuantity = 1;

	// 최대 수량
	int32 MaxQuantity = 1;

	// 수량 표시 텍스트와 버튼 상태를 갱신한다.
	void RefreshQuantityDisplay();

	// 감소 버튼 처리
	UFUNCTION()
	void HandleMinusClicked();

	// 증가 버튼 처리
	UFUNCTION()
	void HandlePlusClicked();

	// 확인 버튼 처리
	UFUNCTION()
	void HandleConfirmClicked();

	// 취소 버튼 처리
	UFUNCTION()
	void HandleCancelClicked();
};
