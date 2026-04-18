#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDCraftTreeWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UEDCraftTreeNodeWidget;
class UEDInventoryComponent;
class UPanelWidget;

/**
 * 선택한 결과 아이템 하나를 기준으로 제작 트리와 연결선을 표시하는 전용 위젯
 */
UCLASS()
class ETERNALDREAMS_API UEDCraftTreeWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 트리 계산에 사용할 인벤토리 컴포넌트를 지정
	void SetInventoryComponent(UEDInventoryComponent* InInventoryComponent);

	// 현재 화면에 표시할 결과 아이템 ID를 지정
	void SetDisplayedResultItemId(const FPrimaryAssetId& InResultItemId);

	// 현재 설정된 인벤토리와 결과 아이템 기준으로 트리를 다시 생성
	void RefreshTree();

protected:
	// 트리 노드가 배치될 컨테이너
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UPanelWidget> CraftTreeContainer;

	// 트리 연결선을 배치할 캔버스
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Craft")
	TObjectPtr<UCanvasPanel> CraftTreeLineCanvas;

	// 생성할 제작 트리 노드 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Craft")
	TSubclassOf<UEDCraftTreeNodeWidget> CraftTreeNodeWidgetClass;

private:
	// 현재 결과 아이템 기준으로 트리 노드 위젯을 다시 생성
	void RebuildCraftTreeNodes();

	// 현재 생성된 노드 위치를 기준으로 연결선을 다시 생성
	void RebuildCraftTreeLines();

	// 두 점 사이에 가로 또는 세로 선분 위젯 하나를 추가
	void AddCraftTreeLineSegment(const FVector2D& StartPoint, const FVector2D& EndPoint, const FLinearColor& LineColor, float LineThickness);

	// 다음 Tick에서 연결선을 다시 계산하도록 dirty 상태로 표시
	void MarkCraftTreeLinesDirty();

	// 현재 노드 배치 상태를 해시로 계산해 레이아웃 변경 여부를 감지
	uint32 BuildCraftTreeLayoutHash() const;

private:
	// 트리 계산에 사용할 인벤토리 컴포넌트
	UPROPERTY(Transient)
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 화면에 생성된 트리 노드 위젯 인스턴스들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgets;

	// BuildCraftTreeFlat()으로 얻은 원본 트리 노드 데이터
	UPROPERTY(Transient)
	TArray<FEDCraftTreeFlatNode> CachedFlatTreeNodes;

	// 노드 ID와 실제 생성된 위젯을 연결하는 맵
	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UEDCraftTreeNodeWidget>> CraftTreeNodeWidgetMap;

	// 캔버스에 생성한 연결선 위젯들
	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> CraftTreeLineWidgets;

	// 현재 화면에 표시 중인 결과 아이템 ID
	FPrimaryAssetId DisplayedResultItemId;

	// 다음 Tick에서 연결선 재계산이 필요한지 여부
	bool bCraftTreeLinesDirty = true;

	// 마지막으로 연결선을 만들 때 기준이 된 선 캔버스 크기
	FVector2D LastCraftTreeLineCanvasSize = FVector2D::ZeroVector;

	// 마지막으로 연결선을 만들 때 기준이 된 트리 컨텐츠 크기
	FVector2D LastCraftTreeContentSize = FVector2D::ZeroVector;

	// 마지막으로 계산한 노드 배치 해시값
	uint32 LastCraftTreeLayoutHash = 0;
};
