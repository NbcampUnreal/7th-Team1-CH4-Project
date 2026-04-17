#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
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
 * 현재 제작 가능한 아이템 목록과 제작 트리를 보여주는 위젯
 * 아이템 사전처럼 정보를 보여주고, 핫키 입력 시 목록의 첫 번째 제작 가능 아이템을 제작
 */
UCLASS()
class ETERNALDREAMS_API UEDItemCraftingWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UEDItemCraftingWidget();
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	// 외부에서 현재 플레이어 인벤토리를 직접 지정
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void SetInventoryComponent(UEDInventoryComponent* InInventoryComponent);

	// 현재 제작 가능한 레시피 목록의 첫 번째 아이템을 기준으로 제작을 요청
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

	// 레시피 카드 목록이 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> RecipeListContainer;

	// 제작 트리 노드가 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> CraftTreeContainer;

	// 제작 트리 연결선을 배치할 캔버스
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UCanvasPanel> CraftTreeLineCanvas;

	// 현재 대표 제작 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> SelectedRecipeNameText;

	// 현재 대표 제작 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> SelectedRecipeIconImage;

	// 현재 대표 제작 아이템 상태 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> SelectedRecipeStateText;

	// 생성할 레시피 카드 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftRecipeEntryWidget> RecipeEntryWidgetClass;

	// 생성할 제작 트리 노드 위젯 클래스
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

	// 현재 카테고리를 기준으로 제작 가능한 레시피 목록을 새로 구성
	void RefreshCraftRecipes();

	// 현재 선택된 카테고리에 포함되는 레시피인지 확인
	bool IsRecipeInSelectedCategory(const FEDCraftableRecipeEntry& InRecipeData) const;

	// 레시피 카드 목록을 다시 생성
	void RebuildRecipeEntries();

	// 현재 첫 번째 제작 가능 아이템 기준으로 제작 트리 노드를 다시 생성
	void RebuildCraftTreeNodes();

	// 현재 트리 노드 배치에 맞춰 연결선을 다시 생성
	void RebuildCraftTreeLines();

	// 연결선 한 구간을 얇은 사각형 위젯으로 캔버스에 추가
	void AddCraftTreeLineSegment(const FVector2D& StartPoint, const FVector2D& EndPoint, const FLinearColor& LineColor, float LineThickness);

	// 다음 Tick에서 연결선을 다시 계산하도록 표시
	void MarkCraftTreeLinesDirty();

	// 노드 위치와 크기를 바탕으로 현재 트리 배치 상태 해시를 계산
	uint32 BuildCraftTreeLayoutHash() const;

	// 상단 요약 패널을 현재 첫 번째 제작 가능 아이템 기준으로 갱신
	void RefreshSelectedRecipeSummary();

	// 현재 캐시된 제작 가능 레시피 중 첫 번째 엔트리를 가져옴
	bool TryGetFirstCraftableRecipeEntry(FEDCraftableRecipeEntry& OutRecipeData) const;

	// 레시피 카드와 요약 UI에 필요한 결과 아이템 표시 정보를 꺼냄
	bool ResolveRecipeDisplayData(const FEDCraftableRecipeEntry& InRecipeData, UTexture2D*& OutIconTexture, FPrimaryAssetId& OutResultItemId) const;

	UFUNCTION()
	void HandleWeaponCategoryClicked();

	UFUNCTION()
	void HandleTopArmorCategoryClicked();

	UFUNCTION()
	void HandleBottomArmorCategoryClicked();

	UFUNCTION()
	void HandleInventoryChanged();

private:
	// 제작 가능 여부 계산과 제작 요청에 사용하는 플레이어 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 현재 카테고리 기준으로 캐시된 제작 가능 레시피 목록
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftableRecipeEntry> CachedRecipeEntries;

	// 레시피 목록 영역에 생성된 카드 위젯들
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftRecipeEntryWidget>> RecipeEntryWidgets;

	// 제작 트리 영역에 생성된 노드 위젯들
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgets;

	// 현재 대표 아이템의 평면 트리 데이터
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftTreeFlatNode> CachedFlatTreeNodes;

	// 노드 ID와 실제 생성된 트리 노드 위젯을 연결하는 맵
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TMap<int32, TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgetMap;

	// 연결선으로 생성한 사각형 위젯들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> CraftTreeLineWidgets;

	// 현재 선택된 제작 카테고리
	EEDEquippableType SelectedCraftCategory = EEDEquippableType::Weapon;

	// 연결선을 다시 계산해야 하는지 여부
	bool bCraftTreeLinesDirty = true;

	// 마지막으로 계산한 연결선 캔버스 크기
	FVector2D LastCraftTreeLineCanvasSize = FVector2D::ZeroVector;

	// 마지막으로 계산한 트리 콘텐츠 크기
	FVector2D LastCraftTreeContentSize = FVector2D::ZeroVector;

	// 마지막으로 계산한 트리 레이아웃 해시
	uint32 LastCraftTreeLayoutHash = 0;
};
