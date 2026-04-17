#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDItemCraftingWidget.generated.h"

class UPanelWidget;
class UImage;
class UTextBlock;
class UButton;
class UEDInventoryComponent;
class UEDCraftRecipeEntryWidget;
class UEDCraftTreeNodeWidget;
class FPaintArgs;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;

/**
 * 플레이어 화면 우측 상단에 표시되는 제작 메인 위젯
 * 레시피 목록, 선택된 레시피 요약, 성장 트리를 함께 갱신
 */
UCLASS()
class ETERNALDREAMS_API UEDItemCraftingWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	// 위젯 생성 시 인벤토리와 버튼 이벤트를 연결
	virtual void NativeConstruct() override;

	// 위젯 제거 시 바인딩한 이벤트를 정리
	virtual void NativeDestruct() override;

	// 성장 트리 노드 사이의 연결선을 직접 그림
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	// 외부에서 현재 플레이어 인벤토리 컴포넌트를 직접 지정
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void SetInventoryComponent(UEDInventoryComponent* InInventoryComponent);

	// 현재 선택된 레시피를 기준으로 제작 시도
	UFUNCTION(BlueprintCallable, Category = "Craft")
	bool RequestCraftSelectedRecipe();

protected:
	// 무기 카테고리 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UButton> WeaponCategoryButton;

	// 상의 카테고리 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UButton> TopArmorCategoryButton;

	// 하의 카테고리 버튼
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UButton> BottomArmorCategoryButton;

	// 레시피 목록 엔트리가 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> RecipeListContainer;

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

	// 생성할 성장 트리 노드 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftTreeNodeWidget> CraftTreeNodeWidgetClass;

private:
	// 소유 플레이어 Pawn에서 인벤토리 컴포넌트 찾기
	void InitializeInventoryComponent();
	
	// 무기/상의/하의 카테고리 버튼 클릭 이벤트를 연결
	void BindCategoryTabButtons();
	
	// 인벤토리 변경 이벤트를 바인딩
	void BindInventoryChanged();
	
	// 인벤토리 변경 이벤트 바인딩을 해제
	void UnbindInventoryChanged();

	// 현재 인벤토리와 선택 카테고리를 기준으로 제작 레시피 목록을 다시 구성
	void RefreshCraftRecipes();
	
	// 특정 레시피가 현재 선택된 카테고리에 속하는지 확인
	bool IsRecipeInSelectedCategory(const FEDCraftRecipeViewData& InRecipeData) const;
	
	// 레시피 목록 엔트리 UI를 다시 생성
	void RebuildRecipeEntries();
	
	// 선택된 레시피의 성장 트리 노드 UI를 다시 생성
	void RebuildCraftTreeNodes();
	
	// 선택된 레시피의 아이콘/이름/상태 요약을 갱신
	void RefreshSelectedRecipeSummary();

	// 현재 선택된 레시피 뷰 데이터를 찾기
	bool TryGetSelectedRecipeViewData(FEDCraftRecipeViewData& OutRecipeData) const;

	// 플랫 트리 노드 데이터를 UI 표시용 노드 데이터로 변환
	bool BuildCraftTreeNodeViewData(const FEDCraftTreeFlatNode& InFlatNode, FEDCraftTreeNodeViewData& OutNodeData) const;

	// 인벤토리 슬롯과 장비 슬롯을 포함한 총 보유 수량을 계산
	int32 CountOwnedItemQuantity(const FPrimaryAssetId& ItemId) const;

	// 레시피 목록에서 특정 엔트리를 클릭했을 때 선택 상태를 갱신
	void HandleRecipeEntryClicked(FName InRecipeRowId);

	// 무기 카테고리 탭 선택 처리
	UFUNCTION()
	void HandleWeaponCategoryClicked();

	// 상의 카테고리 탭 선택 처리
	UFUNCTION()
	void HandleTopArmorCategoryClicked();

	// 하의 카테고리 탭 선택 처리
	UFUNCTION()
	void HandleBottomArmorCategoryClicked();

	// 인벤토리 데이터가 바뀌었을 때 제작 UI를 다시 갱신
	UFUNCTION()
	void HandleInventoryChanged();

	// 제작 성공 메시지를 표시
	void ShowCraftSuccess(const FText& InMessage) const;

	// 제작 실패 사유를 사용자에게 표시
	void ShowCraftFailure(EEDInventoryActionFailure Failure) const;

	// 결과 메시지 영역을 비움
	void ClearActionMessage() const;

private:
	// 현재 플레이어가 사용하는 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 현재 카테고리 기준으로 캐시된 제작 레시피 목록
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftRecipeViewData> CachedRecipeViews;

	// 생성된 레시피 목록 엔트리 위젯들
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftRecipeEntryWidget>> RecipeEntryWidgets;

	// 생성된 성장 트리 노드 위젯들
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgets;

	// 현재 선택된 레시피의 플랫 트리 원본 데이터
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftTreeFlatNode> CachedFlatTreeNodes;

	// NodeId로 트리 노드 위젯을 빠르게 찾기 위한 맵
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TMap<int32, TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgetMap;

	// 현재 선택된 장비 카테고리
	EEDEquippableType SelectedCraftCategory = EEDEquippableType::Weapon;

	// 현재 선택된 레시피의 RowId
	FName SelectedRecipeRowId = NAME_None;
};
