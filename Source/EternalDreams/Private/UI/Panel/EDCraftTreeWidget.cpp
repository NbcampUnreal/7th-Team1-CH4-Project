// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDCraftTreeWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/PanelWidget.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/HUD/EDCraftTreeNodeWidget.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveCraftTreeItemData(const FPrimaryAssetId& ItemId)
{
	if (!ItemId.IsValid())
	{
		return nullptr;
	}

	UObject* ItemObject = UAssetManager::Get().GetPrimaryAssetObject(ItemId);
	if (!ItemObject)
	{
		const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
		if (AssetPath.IsValid())
		{
			ItemObject = AssetPath.TryLoad();
		}
	}

	return Cast<UEDInventoryItemDataAsset>(ItemObject);
}
}

void UEDCraftTreeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CraftTreeLineCanvas || !CraftTreeContainer)
	{
		return;
	}

	const FVector2D CurrentLineCanvasSize = CraftTreeLineCanvas->GetCachedGeometry().GetLocalSize();
	const FVector2D CurrentTreeContentSize = CraftTreeContainer->GetCachedGeometry().GetLocalSize();
	const uint32 CurrentLayoutHash = BuildCraftTreeLayoutHash();

	// 연결선은 노드가 실제 배치된 뒤의 좌표를 기준으로 다시 계산
	// 창 크기, 컨텐츠 크기, 노드 정렬 결과 중 하나라도 바뀌면 선을 다시 만듦
	const bool bCanvasSizeChanged = !CurrentLineCanvasSize.Equals(LastCraftTreeLineCanvasSize, 0.5f);
	const bool bContentSizeChanged = !CurrentTreeContentSize.Equals(LastCraftTreeContentSize, 0.5f);
	const bool bLayoutChanged = CurrentLayoutHash != LastCraftTreeLayoutHash;

	if (bCraftTreeLinesDirty || bCanvasSizeChanged || bContentSizeChanged || bLayoutChanged)
	{
		RebuildCraftTreeLines();
		LastCraftTreeLineCanvasSize = CurrentLineCanvasSize;
		LastCraftTreeContentSize = CurrentTreeContentSize;
		LastCraftTreeLayoutHash = CurrentLayoutHash;
		bCraftTreeLinesDirty = false;
	}
}

void UEDCraftTreeWidget::SetInventoryComponent(UEDInventoryComponent* InInventoryComponent)
{
	if (InventoryComponent == InInventoryComponent)
	{
		return;
	}

	InventoryComponent = InInventoryComponent;
	RefreshTree();
}

void UEDCraftTreeWidget::SetDisplayedResultItemId(const FPrimaryAssetId& InResultItemId)
{
	if (DisplayedResultItemId == InResultItemId)
	{
		return;
	}

	DisplayedResultItemId = InResultItemId;
	RefreshTree();
}

void UEDCraftTreeWidget::RefreshTree()
{
	UE_LOG(LogTemp, Warning, TEXT("CraftTreeWidget: RefreshTree Inventory=%s ResultItem=%s Container=%s NodeClass=%s"),
		InventoryComponent ? TEXT("Valid") : TEXT("Null"),
		*DisplayedResultItemId.ToString(),
		CraftTreeContainer ? TEXT("Valid") : TEXT("Null"),
		CraftTreeNodeWidgetClass ? *CraftTreeNodeWidgetClass->GetName() : TEXT("None"));

	RebuildCraftTreeNodes();
}

void UEDCraftTreeWidget::RebuildCraftTreeNodes()
{
	const FMargin TreeRowPadding(0.0f, 8.0f, 0.0f, 8.0f);
	const FMargin TreeNodePadding(10.0f, 4.0f, 10.0f, 4.0f);

	if (!CraftTreeContainer)
	{
		return;
	}

	CraftTreeContainer->ClearChildren();
	CraftTreeNodeWidgets.Reset();
	CachedFlatTreeNodes.Reset();
	CraftTreeNodeWidgetMap.Reset();
	MarkCraftTreeLinesDirty();

	if (!CraftTreeNodeWidgetClass || !InventoryComponent || !DisplayedResultItemId.IsValid())
	{
		return;
	}

	bool bCyclePruned = false;
	UEDInventoryBlueprintLibrary::BuildCraftTreeFlat(
		InventoryComponent,
		DisplayedResultItemId,
		CachedFlatTreeNodes,
		bCyclePruned,
		8);

	UE_LOG(LogTemp, Warning, TEXT("CraftTreeWidget: BuildCraftTreeFlat ResultItem=%s Nodes=%d CyclePruned=%s"),
		*DisplayedResultItemId.ToString(),
		CachedFlatTreeNodes.Num(),
		bCyclePruned ? TEXT("true") : TEXT("false"));

	if (CachedFlatTreeNodes.Num() <= 0 || !WidgetTree)
	{
		return;
	}

	CachedFlatTreeNodes.Sort([](const FEDCraftTreeFlatNode& A, const FEDCraftTreeFlatNode& B)
	{
		if (A.Depth != B.Depth)
		{
			return A.Depth < B.Depth;
		}

		return A.NodeId < B.NodeId;
	});

	int32 CurrentDepth = INDEX_NONE;
	UHorizontalBox* CurrentRow = nullptr;

	for (const FEDCraftTreeFlatNode& FlatNode : CachedFlatTreeNodes)
	{
		// Flat 트리 데이터는 depth만 들고 있으므로,
		// 같은 depth끼리 한 줄에 배치하고 depth가 바뀌면 새 row를 만듦
		if (CurrentDepth != FlatNode.Depth)
		{
			CurrentDepth = FlatNode.Depth;
			CurrentRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
			if (!CurrentRow)
			{
				continue;
			}

			if (UPanelSlot* RowPanelSlot = CraftTreeContainer->AddChild(CurrentRow))
			{
				if (UVerticalBoxSlot* RowSlot = Cast<UVerticalBoxSlot>(RowPanelSlot))
				{
					RowSlot->SetPadding(TreeRowPadding);
					RowSlot->SetHorizontalAlignment(HAlign_Center);
				}
			}
		}

		if (!CurrentRow)
		{
			continue;
		}

		const UEDInventoryItemDataAsset* ItemData = ResolveCraftTreeItemData(FlatNode.ItemId);
		const FText DisplayName = ItemData && !ItemData->DisplayName.IsEmpty()
			? ItemData->DisplayName
			: FText::FromName(FlatNode.ItemId.PrimaryAssetName);
		const EEDItemRarity Rarity = ItemData ? ItemData->Rarity : EEDItemRarity::Normal;
		UTexture2D* IconTexture = ItemData ? ItemData->IconTexture : nullptr;

		UEDCraftTreeNodeWidget* NodeWidget = CreateWidget<UEDCraftTreeNodeWidget>(this, CraftTreeNodeWidgetClass);
		if (!NodeWidget)
		{
			continue;
		}

		FEDCraftTreeNodeDisplayData DisplayData;
		DisplayData.DisplayName = DisplayName;
		DisplayData.IconTexture = IconTexture;
		DisplayData.Rarity = Rarity;
		DisplayData.RequiredQuantity = FlatNode.Quantity;

		NodeWidget->SetTreeNodeDisplayData(DisplayData);
		if (UHorizontalBoxSlot* NodeSlot = CurrentRow->AddChildToHorizontalBox(NodeWidget))
		{
			NodeSlot->SetPadding(TreeNodePadding);
			NodeSlot->SetHorizontalAlignment(HAlign_Center);
			NodeSlot->SetVerticalAlignment(VAlign_Center);
		}

		CraftTreeNodeWidgets.Add(NodeWidget);
		CraftTreeNodeWidgetMap.Add(FlatNode.NodeId, NodeWidget);
	}

	MarkCraftTreeLinesDirty();
}

void UEDCraftTreeWidget::RebuildCraftTreeLines()
{
	CraftTreeLineWidgets.Reset();

	if (!CraftTreeLineCanvas)
	{
		return;
	}

	CraftTreeLineCanvas->ClearChildren();

	if (CachedFlatTreeNodes.Num() <= 0 || CraftTreeNodeWidgetMap.Num() <= 0 || !WidgetTree)
	{
		return;
	}

	const FGeometry LineCanvasGeometry = CraftTreeLineCanvas->GetCachedGeometry();
	if (LineCanvasGeometry.GetLocalSize().IsNearlyZero())
	{
		return;
	}

	const FLinearColor LineColor(0.42f, 0.36f, 0.29f, 1.0f);
	const float LineThickness = 2.0f;
	const float ParentBottomInset = 3.0f;
	const float ChildTopInset = 3.0f;
	const float ParentStemLength = 12.0f;
	const float ChildStemLength = 12.0f;

	// 각 FlatNode를 돌면서 "부모 -> 자식" 관계가 있는 노드 쌍만 연결선을 만듦
	for (const FEDCraftTreeFlatNode& FlatNode : CachedFlatTreeNodes)
	{
		if (FlatNode.ParentNodeId < 0)
		{
			continue;
		}

		const TObjectPtr<UEDCraftTreeNodeWidget>* ParentWidgetPtr = CraftTreeNodeWidgetMap.Find(FlatNode.ParentNodeId);
		const TObjectPtr<UEDCraftTreeNodeWidget>* ChildWidgetPtr = CraftTreeNodeWidgetMap.Find(FlatNode.NodeId);
		if (!ParentWidgetPtr || !ChildWidgetPtr || !(*ParentWidgetPtr) || !(*ChildWidgetPtr))
		{
			continue;
		}

		const FGeometry ParentGeometry = (*ParentWidgetPtr)->GetCachedGeometry();
		const FGeometry ChildGeometry = (*ChildWidgetPtr)->GetCachedGeometry();
		if (ParentGeometry.GetLocalSize().IsNearlyZero() || ChildGeometry.GetLocalSize().IsNearlyZero())
		{
			continue;
		}

		// 각 노드 위젯 안에서 부모는 "아래 중앙", 자식은 "위 중앙" 지점을 연결 시작점으로 사용
		// 먼저 각 위젯 좌표계에서 절대 좌표로 변환
		const FVector2D ParentBottomAbsolute = ParentGeometry.LocalToAbsolute(
			FVector2D(ParentGeometry.GetLocalSize().X * 0.5f, ParentGeometry.GetLocalSize().Y - ParentBottomInset));
		const FVector2D ChildTopAbsolute = ChildGeometry.LocalToAbsolute(
			FVector2D(ChildGeometry.GetLocalSize().X * 0.5f, ChildTopInset));

		// 실제 선은 LineCanvas 위에 배치되므로,
		// 절대 좌표를 다시 "라인 캔버스 기준 로컬 좌표"로 변환
		const FVector2D ParentBottomLocal = LineCanvasGeometry.AbsoluteToLocal(ParentBottomAbsolute);
		const FVector2D ChildTopLocal = LineCanvasGeometry.AbsoluteToLocal(ChildTopAbsolute);
		if (ChildTopLocal.Y <= ParentBottomLocal.Y)
		{
			continue;
		}

		// MidY - 가로선이 지나갈 높이
		// 부모에서 조금 내려온 지점과 자식에서 조금 올라온 지점 사이에서 결정
		float MidY = FMath::Min(
			ParentBottomLocal.Y + ParentStemLength,
			ChildTopLocal.Y - ChildStemLength);

		// 두 노드 간 간격이 너무 좁으면 중간 높이를 써서 선이 뒤집히지 않게 막음
		if (MidY <= ParentBottomLocal.Y || MidY >= ChildTopLocal.Y)
		{
			MidY = ParentBottomLocal.Y + ((ChildTopLocal.Y - ParentBottomLocal.Y) * 0.5f);
		}

		AddCraftTreeLineSegment(ParentBottomLocal, FVector2D(ParentBottomLocal.X, MidY), LineColor, LineThickness);
		AddCraftTreeLineSegment(FVector2D(ParentBottomLocal.X, MidY), FVector2D(ChildTopLocal.X, MidY), LineColor, LineThickness);
		AddCraftTreeLineSegment(FVector2D(ChildTopLocal.X, MidY), ChildTopLocal, LineColor, LineThickness);
	}
}

void UEDCraftTreeWidget::AddCraftTreeLineSegment(
	const FVector2D& StartPoint,
	const FVector2D& EndPoint,
	const FLinearColor& LineColor,
	float LineThickness)
{
	if (!CraftTreeLineCanvas || !WidgetTree)
	{
		return;
	}

	const FVector2D Delta = EndPoint - StartPoint;
	const bool bIsHorizontal = FMath::Abs(Delta.X) >= FMath::Abs(Delta.Y);

	FVector2D SegmentPosition;
	FVector2D SegmentSize;

	// 선 위젯은 얇은 사각형 Border를 가로 또는 세로로 배치해서 만듦
	// 따라서 먼저 현재 구간이 가로선인지 세로선인지 판단하고 위치와 크기를 계산
	if (bIsHorizontal)
	{
		SegmentPosition.X = FMath::Min(StartPoint.X, EndPoint.X);
		SegmentPosition.Y = StartPoint.Y - (LineThickness * 0.5f);
		SegmentSize.X = FMath::Abs(Delta.X);
		SegmentSize.Y = LineThickness;
	}
	else
	{
		SegmentPosition.X = StartPoint.X - (LineThickness * 0.5f);
		SegmentPosition.Y = FMath::Min(StartPoint.Y, EndPoint.Y);
		SegmentSize.X = LineThickness;
		SegmentSize.Y = FMath::Abs(Delta.Y);
	}

	if (SegmentSize.X <= KINDA_SMALL_NUMBER || SegmentSize.Y <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	UBorder* LineWidget = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	if (!LineWidget)
	{
		return;
	}

	LineWidget->SetBrushColor(LineColor);

	if (UCanvasPanelSlot* LineSlot = CraftTreeLineCanvas->AddChildToCanvas(LineWidget))
	{
		LineSlot->SetAutoSize(false);
		LineSlot->SetPosition(SegmentPosition);
		LineSlot->SetSize(SegmentSize);
	}

	CraftTreeLineWidgets.Add(LineWidget);
}

void UEDCraftTreeWidget::MarkCraftTreeLinesDirty()
{
	// 트리 노드 배치가 바뀌면 Tick에서 연결선을 다시 만들 수 있도록
	// 이전 프레임 기준 캐시를 초기화
	bCraftTreeLinesDirty = true;
	LastCraftTreeLineCanvasSize = FVector2D::ZeroVector;
	LastCraftTreeContentSize = FVector2D::ZeroVector;
	LastCraftTreeLayoutHash = 0;
}

uint32 UEDCraftTreeWidget::BuildCraftTreeLayoutHash() const
{
	if (!CraftTreeLineCanvas || CraftTreeNodeWidgetMap.Num() <= 0)
	{
		return 0;
	}

	const FGeometry LineCanvasGeometry = CraftTreeLineCanvas->GetCachedGeometry();
	if (LineCanvasGeometry.GetLocalSize().IsNearlyZero())
	{
		return 0;
	}

	TArray<int32> NodeIds;
	CraftTreeNodeWidgetMap.GenerateKeyArray(NodeIds);
	NodeIds.Sort();

	uint32 LayoutHash = 0;

	// "노드가 어디에 배치됐는지"를 숫자 해시로 요약
	// 창 크기는 같아도 정렬 결과가 달라질 수 있으므로,
	// 각 노드의 중심 좌표와 크기를 함께 반영
	for (const int32 NodeId : NodeIds)
	{
		const TObjectPtr<UEDCraftTreeNodeWidget>* NodeWidgetPtr = CraftTreeNodeWidgetMap.Find(NodeId);
		if (!NodeWidgetPtr || !(*NodeWidgetPtr))
		{
			continue;
		}

		const FGeometry NodeGeometry = (*NodeWidgetPtr)->GetCachedGeometry();
		if (NodeGeometry.GetLocalSize().IsNearlyZero())
		{
			continue;
		}

		const FVector2D NodeCenterAbsolute = NodeGeometry.LocalToAbsolute(NodeGeometry.GetLocalSize() * 0.5f);
		const FVector2D NodeCenterLocal = LineCanvasGeometry.AbsoluteToLocal(NodeCenterAbsolute);
		const FVector2D NodeSize = NodeGeometry.GetLocalSize();

		// 실수 좌표를 그대로 비교하면 미세한 흔들림에도 과하게 다시 계산할 수 있으므로
		// 적당히 반올림한 정수값만 해시에 넣음
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(NodeId));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeCenterLocal.X * 10.0f)));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeCenterLocal.Y * 10.0f)));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeSize.X * 10.0f)));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeSize.Y * 10.0f)));
	}

	return LayoutHash;
}
