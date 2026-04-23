#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDItemCraftingWidget.generated.h"

class UButton;
class UDataTable;
class UImage;
class UPanelWidget;
class UTextBlock;
class UEDCraftRecipeEntryWidget;
class UEDCraftTreeWidget;
class UEDInventoryComponent;

/**
 * 전체 제작 레시피 목록과 선택한 레시피 상세 정보를 보여주는 제작 패널
 * 실제 제작 실행은 현재 제작 가능한 레시피 캐시의 첫 번째 항목을 기준으로 처리
 */
UCLASS()
class ETERNALDREAMS_API UEDItemCraftingWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UEDItemCraftingWidget();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	// 외부에서 현재 플레이어 인벤토리를 직접 지정
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void SetInventoryComponent(UEDInventoryComponent* InInventoryComponent);

	// 현재 제작 가능한 레시피 목록의 첫 번째 아이템을 제작 요청
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

	// 제작 트리 전용 위젯
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UEDCraftTreeWidget> CraftTreeWidget;

	// 현재 선택한 제작 아이템 이름 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UTextBlock> SelectedRecipeNameText;

	// 현재 선택한 제작 아이템 아이콘
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UImage> SelectedRecipeIconImage;
	
	// 생성할 레시피 카드 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftRecipeEntryWidget> RecipeEntryWidgetClass;

private:
	// 소유 플레이어에서 인벤토리 컴포넌트 찾기
	void InitializeInventoryComponent();

	// 카테고리 버튼 클릭 이벤트 바인딩
	void BindCategoryTabButtons();

	// 인벤토리 변경 이벤트 바인딩
	void BindInventoryChanged();

	// 인벤토리 변경 이벤트 바인딩 해제
	void UnbindInventoryChanged();

	// 현재 카테고리를 기준으로 전체 레시피 목록을 다시 구성
	void RefreshCraftRecipes();

	// 제작 테이블 전체를 읽어 표시용 레시피 목록을 구성
	void GatherAllRecipeEntries(TArray<FEDCraftableRecipeEntry>& OutRecipeEntries) const;

	// 현재 선택한 카테고리에 포함되는 레시피인지 확인
	bool IsRecipeInSelectedCategory(const FEDCraftableRecipeEntry& InRecipeData) const;

	// 결과 아이템 데이터에서 장비 카테고리를 추출
	EEDEquippableType ResolveRecipeCategory(const FEDCraftableRecipeEntry& InRecipeData) const;

	// 레시피 카드 목록을 다시 생성
	void RebuildRecipeEntries();

	// 상단 요약 패널을 현재 표시 대상 레시피 기준으로 갱신
	void RefreshSelectedRecipeSummary();

	// 현재 패널에서 상세 보기 대상으로 사용하는 레시피를 반환
	bool TryGetDisplayedRecipeEntry(FEDCraftableRecipeEntry& OutRecipeData) const;

	// 현재 제작 가능한 레시피 캐시 중 첫 번째 항목을 반환
	bool TryGetFirstCraftableRecipeEntry(FEDCraftableRecipeEntry& OutRecipeData) const;

	// 레시피 카드와 요약 UI에 필요한 결과 아이템 표시 정보를 계산
	bool ResolveRecipeDisplayData(const FEDCraftableRecipeEntry& InRecipeData, UTexture2D*& OutIconTexture, FPrimaryAssetId& OutResultItemId) const;

	// 무기 카테고리 버튼 클릭 이벤트 처리
	UFUNCTION()
	void HandleWeaponCategoryClicked();

	// 상의 카테고리 버튼 클릭 이벤트 처리
	UFUNCTION()
	void HandleTopArmorCategoryClicked();

	// 하의 카테고리 버튼 클릭 이벤트 처리
	UFUNCTION()
	void HandleBottomArmorCategoryClicked();

	// 인벤토리 변경 시 갱신 처리
	UFUNCTION()
	void HandleInventoryChanged();

	// 레시피 카드 클릭 시 상세 보기 대상을 변경
	void HandleRecipeEntryClicked(FName InRecipeRowId);

private:
	// 레시피 표시와 제작 요청에 사용할 플레이어 인벤토리 컴포넌트
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 현재 카테고리 기준으로 캐시된 전체 레시피 목록
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<FEDCraftableRecipeEntry> CachedRecipeEntries;

	// 레시피 목록 영역에 생성된 카드 위젯들
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Craft", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UEDCraftRecipeEntryWidget>> RecipeEntryWidgets;

	// 현재 선택한 제작 카테고리
	EEDEquippableType SelectedCraftCategory = EEDEquippableType::Weapon;

	// 현재 패널에서 상세 보기 중인 레시피 RowId
	FName DisplayedRecipeRowId = NAME_None;
};
