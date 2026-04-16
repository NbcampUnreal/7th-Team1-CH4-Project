#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDItemCraftingWidget.generated.h"

class UBorder;
class UButton;
class UCanvasPanel;
class UImage;
class UPanelWidget;
class UTextBlock;
class UEDCraftRecipeEntryWidget;
class UEDCraftTreeNodeWidget;
class UEDInventoryComponent;
class FPaintArgs;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;

/**
 * 플레이어 화면 우측 상단에 표시되는 제작 메인 위젯
 * 레시피 목록, 선택된 레시피 요약, 성장 트리와 연결선을 함께 갱신
 */
UCLASS()
class ETERNALDREAMS_API UEDItemCraftingWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
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

	// 현재 선택된 레시피 기준으로 제작 시도
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

	// 레시피 카드가 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> RecipeListContainer;

	// 성장 트리 노드가 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> CraftTreeContainer;

	// 성장 트리 연결선을 배치할 캔버스 레이어
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UCanvasPanel> CraftTreeLineCanvas;

	// 현재 선택된 결과 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> SelectedRecipeNameText;

	// 현재 선택된 결과 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> SelectedRecipeIconImage;

	// 현재 선택된 제작 상태 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> SelectedRecipeStateText;

	// 성공/실패 메시지 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> ActionResultText;

	// 레시피 카드 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftRecipeEntryWidget> RecipeEntryWidgetClass;

	// 성장 트리 노드 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftTreeNodeWidget> CraftTreeNodeWidgetClass;

private:
	// 소유 플레이어에서 인벤토리 컴포넌트 찾기
	void InitializeInventoryComponent();

	// 카테고리 버튼 클릭 이벤트를 바인딩
	void BindCategoryTabButtons();

	// 인벤토리 변경 이벤트를 바인딩
	void BindInventoryChanged();

	// 인벤토리 변경 이벤트 바인딩을 해제
	void UnbindInventoryChanged();

	// 현재 인벤토리 상태를 기준으로 제작 HUD 전체를 새로 구성
	void RefreshCraftRecipes();

	// 현재 선택된 장비 카테고리에 속하는 레시피인지 확인
	bool IsRecipeInSelectedCategory(const FEDCraftRecipeViewData& InRecipeData) const;

	// 왼쪽 레시피 카드 목록을 다시 만듦
	void RebuildRecipeEntries();

	// 선택된 레시피의 성장 트리 노드를 다시 만듦
	void RebuildCraftTreeNodes();

	// 성장 트리 노드 배치 결과를 기준으로 연결선을 다시 만듦
	void RebuildCraftTreeLines();

	// 한 구간의 직선을 캔버스 레이어에 배치
	void AddCraftTreeLineSegment(const FVector2D& StartPoint, const FVector2D& EndPoint, const FLinearColor& LineColor, float LineThickness);

	// 연결선이 다시 계산되어야 함을 표시
	void MarkCraftTreeLinesDirty();

	// 현재 트리 노드 배치 상태를 해시로 계산
	uint32 BuildCraftTreeLayoutHash() const;

	// 선택된 레시피 요약 텍스트와 아이콘을 갱신
	void RefreshSelectedRecipeSummary();

	// 현재 선택된 레시피 뷰 데이터를 가져옴
	bool TryGetSelectedRecipeViewData(FEDCraftRecipeViewData& OutRecipeData) const;

	// Flat 트리 노드를 UI용 트리 노드 데이터로 변환
	bool BuildCraftTreeNodeViewData(const FEDCraftTreeFlatNode& InFlatNode, FEDCraftTreeNodeViewData& OutNodeData) const;

	// 현재 플레이어가 보유한 특정 아이템 수량을 계산
	int32 CountOwnedItemQuantity(const FPrimaryAssetId& ItemId) const;

	// 레시피 카드를 클릭했을 때 선택 레시피를 바꿈
	void HandleRecipeEntryClicked(FName InRecipeRowId);

	// 무기 카테고리 탭을 눌렀을 때 호출
	// 제작 레시피 목록을 무기 레시피만 보이도록 재구성
	UFUNCTION()
	void HandleWeaponCategoryClicked();

	// 상의 카테고리 탭을 눌렀을 때 호출
	// 제작 레시피 목록을 상의 레시피만 보이도록 재구성
	UFUNCTION()
	void HandleTopArmorCategoryClicked();

	// 하의 카테고리 탭을 눌렀을 때 호출
	// 제작 레시피 목록을 하의 레시피만 보이도록 재구성
	UFUNCTION()
	void HandleBottomArmorCategoryClicked();

	// 플레이어 인벤토리 내용이 바뀌었을 때 호출
	// 제작 가능 여부, 레시피 목록, 성장 트리를 현재 상태에 맞게 다시 갱신
	UFUNCTION()
	void HandleInventoryChanged();

	// 성공 메시지를 표시
	void ShowCraftSuccess(const FText& InMessage) const;

	// 실패 사유에 맞는 메시지를 표시
	void ShowCraftFailure(EEDInventoryActionFailure Failure) const;

	// 결과 메시지를 숨김
	void ClearActionMessage() const;
	
	// 제작 HUD가 참조하는 플레이어 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 현재 선택된 카테고리 기준으로 화면에 표시할 레시피 목록 캐시
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftRecipeViewData> CachedRecipeViews;

	// 왼쪽 레시피 카드 목록에 생성된 위젯 인스턴스들
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftRecipeEntryWidget>> RecipeEntryWidgets;

	// 성장 트리 영역에 생성된 노드 위젯 인스턴스들
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgets;

	// 현재 선택된 레시피를 기준으로 만든 성장 트리의 원본 노드 데이터
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftTreeFlatNode> CachedFlatTreeNodes;

	// 트리 노드 ID와 실제 생성된 노드 위젯을 연결하는 맵
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TMap<int32, TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgetMap;

	// 성장 트리 연결선으로 생성하 선 위젯들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> CraftTreeLineWidgets;

	// 현재 선택된 장비 카테고리
	EEDEquippableType SelectedCraftCategory = EEDEquippableType::Weapon;

	// 현재 선택된 레시피 Row 이름
	FName SelectedRecipeRowId = NAME_None;

	// 연결선을 다시 계산해야 하는지 여부
	bool bCraftTreeLinesDirty = true;

	// 마지막으로 연결선을 계산할 때 사용한 캔버스 크기
	FVector2D LastCraftTreeLineCanvasSize = FVector2D::ZeroVector;

	// 마지막으로 연결선을 계산할 때 사용한 트리 컨테이너 크기
	FVector2D LastCraftTreeContentSize = FVector2D::ZeroVector;

	// 마지막으로 연결선을 계산할 때 사용한 노드 배치 해시
	uint32 LastCraftTreeLayoutHash = 0;
};
