#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDItemCraftingWidget.generated.h"

class UPanelWidget;
class UImage;
class UTextBlock;
class UEDInventoryComponent;
class UEDCraftRecipeEntryWidget;
class UEDCraftIngredientEntryWidget;
class UEDCraftTreeNodeWidget;

/**
 * 플레이어 화면 우측 상단에 표시되는 제작 메인 위젯
 * 레시피 목록, 선택된 레시피 요약, 재료 목록, 성장 트리를 함께 갱신
 */
UCLASS()
class ETERNALDREAMS_API UEDItemCraftingWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 외부에서 현재 플레이어 인벤토리를 직접 지정
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void SetInventoryComponent(UEDInventoryComponent* InInventoryComponent);

	// 현재 선택된 레시피로 아이템 제작 요청
	UFUNCTION(BlueprintCallable, Category = "Craft")
	bool RequestCraftSelectedRecipe();

protected:
	// 레시피 목록 엔트리가 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> RecipeListContainer;

	// 선택된 레시피 재료 목록이 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> IngredientListContainer;

	// 선택된 레시피 성장 트리가 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> CraftTreeContainer;

	// 현재 선택된 레시피 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> SelectedRecipeNameText;

	// 현재 선택된 결과 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> SelectedRecipeIconImage;

	// 현재 선택된 레시피 상태 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> SelectedRecipeStateText;

	// 제작 결과 또는 실패 메시지 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> ActionResultText;

	// 생성할 레시피 엔트리 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftRecipeEntryWidget> RecipeEntryWidgetClass;

	// 생성할 재료 엔트리 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftIngredientEntryWidget> IngredientEntryWidgetClass;

	// 생성할 성장 트리 노드 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftTreeNodeWidget> CraftTreeNodeWidgetClass;

private:
	// 소유 플레이어에서 인벤토리 컴포넌트 찾기
	void InitializeInventoryComponent();

	// 인벤토리 변경 이벤트 바인딩
	void BindInventoryChanged();

	// 인벤토리 변경 이벤트 바인딩 해제
	void UnbindInventoryChanged();

	// 현재 인벤토리 상태를 기준으로 제작 레시피 목록을 새로 구성
	void RefreshCraftRecipes();

	// 레시피 목록 UI를 다시 그림
	void RebuildRecipeEntries();

	// 선택된 레시피의 재료 목록 UI를 다시 그림
	void RebuildIngredientEntries();

	// 선택된 레시피의 성장 트리 UI를 다시 그림
	void RebuildCraftTreeNodes();

	// 선택된 레시피 요약 텍스트를 갱신
	void RefreshSelectedRecipeSummary();

	// 현재 선택된 RowId에 해당하는 레시피 뷰 데이터 찾기
	bool TryGetSelectedRecipeViewData(FEDCraftRecipeViewData& OutRecipeData) const;

	// 제작 트리의 단일 노드를 UI용 뷰 데이터로 변환
	bool BuildCraftTreeNodeViewData(const FEDCraftTreeFlatNode& InFlatNode, FEDCraftTreeNodeViewData& OutNodeData) const;

	// 현재 인벤토리 + 장비 슬롯 기준으로 아이템 총 보유량 계산
	int32 CountOwnedItemQuantity(const FPrimaryAssetId& ItemId) const;

	// 레시피 엔트리를 클릭했을 때 선택 상태를 갱신
	void HandleRecipeEntryClicked(FName InRecipeRowId);

	// 인벤토리 변경 시 제작 UI 전체를 다시 갱신
	UFUNCTION()
	void HandleInventoryChanged();

	// 제작 실패 메시지 표시
	void ShowCraftFailure(EEDInventoryActionFailure Failure) const;

	// 현재 액션 메시지를 지움
	void ClearActionMessage() const;

private:
	// 현재 참조 중인 플레이어 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 현재 UI에 표시 중인 레시피 뷰 데이터 목록
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftRecipeViewData> CachedRecipeViews;

	// 생성된 레시피 엔트리 위젯 목록
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftRecipeEntryWidget>> RecipeEntryWidgets;

	// 생성된 재료 엔트리 위젯 목록
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftIngredientEntryWidget>> IngredientEntryWidgets;

	// 생성된 성장 트리 노드 위젯 목록
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgets;

	// 현재 선택된 레시피 RowId
	FName SelectedRecipeRowId = NAME_None;
};
