// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDItemCraftingWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/AssetManager.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDItemDataRows.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/HUD/EDCraftRecipeEntryWidget.h"
#include "UI/HUD/EDCraftTreeNodeWidget.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"

namespace
{
	const UEDInventoryItemDataAsset* ResolveCraftItemData(const FPrimaryAssetId& ItemId)
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

	FText GetCraftFailureText(EEDInventoryActionFailure Failure)
	{
		switch (Failure)
		{
		case EEDInventoryActionFailure::None:
			return FText::GetEmpty();
		case EEDInventoryActionFailure::InvalidInventory:
			return FText::FromString(TEXT("인벤토리 정보를 찾을 수 없습니다."));
		case EEDInventoryActionFailure::InvalidSlot:
			return FText::FromString(TEXT("잘못된 슬롯입니다."));
		case EEDInventoryActionFailure::EmptySlot:
			return FText::FromString(TEXT("선택한 슬롯이 비어 있습니다."));
		case EEDInventoryActionFailure::InvalidQuantity:
			return FText::FromString(TEXT("수량 정보가 올바르지 않습니다."));
		case EEDInventoryActionFailure::SlotConflict:
			return FText::FromString(TEXT("슬롯 상태가 충돌합니다."));
		case EEDInventoryActionFailure::NoSpace:
			return FText::FromString(TEXT("인벤토리 공간이 부족합니다."));
		case EEDInventoryActionFailure::StackLimit:
			return FText::FromString(TEXT("더 이상 같은 아이템을 넣을 수 없습니다."));
		case EEDInventoryActionFailure::MissingData:
			return FText::FromString(TEXT("아이템 데이터가 없습니다."));
		case EEDInventoryActionFailure::InvalidRecipe:
			return FText::FromString(TEXT("유효하지 않은 레시피입니다."));
		case EEDInventoryActionFailure::MissingIngredient:
			return FText::FromString(TEXT("재료가 부족하여 제작할 수 없습니다."));
		case EEDInventoryActionFailure::NotConsumable:
			return FText::FromString(TEXT("사용할 수 없는 아이템입니다."));
		case EEDInventoryActionFailure::HealthAlreadyFull:
			return FText::FromString(TEXT("이미 체력이 가득 찬 상태입니다."));
		case EEDInventoryActionFailure::EffectApplyFailed:
			return FText::FromString(TEXT("효과 적용에 실패했습니다."));
		default:
			return FText::FromString(TEXT("알 수 없는 이유로 제작에 실패했습니다."));
		}
	}
}

void UEDItemCraftingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeInventoryComponent();
	BindCategoryTabButtons();
	BindInventoryChanged();
	RefreshCraftRecipes();
	MarkCraftTreeLinesDirty();
}

void UEDItemCraftingWidget::NativeDestruct()
{
	UnbindInventoryChanged();

	if (WeaponCategoryButton)
	{
		WeaponCategoryButton->OnClicked.RemoveDynamic(this, &UEDItemCraftingWidget::HandleWeaponCategoryClicked);
	}

	if (TopArmorCategoryButton)
	{
		TopArmorCategoryButton->OnClicked.RemoveDynamic(this, &UEDItemCraftingWidget::HandleTopArmorCategoryClicked);
	}

	if (BottomArmorCategoryButton)
	{
		BottomArmorCategoryButton->OnClicked.RemoveDynamic(
			this, &UEDItemCraftingWidget::HandleBottomArmorCategoryClicked);
	}

	Super::NativeDestruct();
}

void UEDItemCraftingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CraftTreeLineCanvas || !CraftTreeContainer)
	{
		return;
	}

	// 연결선은 노드가 실제로 배치된 뒤의 좌표를 기준으로 다시 만들어야 함
	// 매 Tick마다 "트리 영역 크기"와 "노드 배치 상태"가 바뀌었는지 확인
	const FVector2D CurrentLineCanvasSize = CraftTreeLineCanvas->GetCachedGeometry().GetLocalSize();
	const FVector2D CurrentTreeContentSize = CraftTreeContainer->GetCachedGeometry().GetLocalSize();
	const uint32 CurrentLayoutHash = BuildCraftTreeLayoutHash();

	// 단순히 창 크기만 바뀌는 경우도 있지만,
	// 같은 크기 안에서 정렬 결과만 달라지는 경우도 있어서 둘 다 추적
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

int32 UEDItemCraftingWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
	                          bParentEnabled);
}

void UEDItemCraftingWidget::SetInventoryComponent(UEDInventoryComponent* InInventoryComponent)
{
	if (InventoryComponent == InInventoryComponent)
	{
		RefreshCraftRecipes();
		return;
	}

	UnbindInventoryChanged();
	InventoryComponent = InInventoryComponent;
	BindInventoryChanged();
	RefreshCraftRecipes();
}

bool UEDItemCraftingWidget::RequestCraftSelectedRecipe()
{
	if (!InventoryComponent)
	{
		if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
			{
				UIManageSubsystem->ShowToastMessage(GetCraftFailureText(EEDInventoryActionFailure::InvalidInventory),
				                                    EEDUIMessageType::Error, 3.0f);
			}
		}
		return false;
	}

	InventoryComponent->RefreshCraftableRecipesCache();

	FEDCraftableRecipeEntry CraftTargetRecipe;
	if (!TryGetFirstCraftableRecipeEntry(CraftTargetRecipe))
	{
		if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
			{
				UIManageSubsystem->ShowToastMessage(GetCraftFailureText(EEDInventoryActionFailure::InvalidRecipe),
				                                    EEDUIMessageType::Error, 3.0f);
			}
		}
		return false;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSuccess = InventoryComponent->PredicateCraftItem(CraftTargetRecipe.RecipeId, Failure);
	if (!bSuccess)
	{
		if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
		{
			if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
			{
				UIManageSubsystem->ShowToastMessage(GetCraftFailureText(Failure), EEDUIMessageType::Error, 3.0f);
			}
		}
		return false;
	}

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
		{
			UIManageSubsystem->ShowToastMessage(
				FText::Format(FText::FromString(TEXT("{0} 제작에 성공했습니다.")), CraftTargetRecipe.ResultItemName),
				EEDUIMessageType::Success,
				3.0f);
		}
	}

	return true;
}

void UEDItemCraftingWidget::InitializeInventoryComponent()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		return;
	}

	InventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(OwningPawn);
}

void UEDItemCraftingWidget::BindCategoryTabButtons()
{
	if (WeaponCategoryButton)
	{
		WeaponCategoryButton->OnClicked.RemoveDynamic(this, &UEDItemCraftingWidget::HandleWeaponCategoryClicked);
		WeaponCategoryButton->OnClicked.AddDynamic(this, &UEDItemCraftingWidget::HandleWeaponCategoryClicked);
	}

	if (TopArmorCategoryButton)
	{
		TopArmorCategoryButton->OnClicked.RemoveDynamic(this, &UEDItemCraftingWidget::HandleTopArmorCategoryClicked);
		TopArmorCategoryButton->OnClicked.AddDynamic(this, &UEDItemCraftingWidget::HandleTopArmorCategoryClicked);
	}

	if (BottomArmorCategoryButton)
	{
		BottomArmorCategoryButton->OnClicked.RemoveDynamic(
			this, &UEDItemCraftingWidget::HandleBottomArmorCategoryClicked);
		BottomArmorCategoryButton->OnClicked.AddDynamic(this, &UEDItemCraftingWidget::HandleBottomArmorCategoryClicked);
	}
}

void UEDItemCraftingWidget::BindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &UEDItemCraftingWidget::HandleInventoryChanged);
}

void UEDItemCraftingWidget::UnbindInventoryChanged()
{
	if (!InventoryComponent)
	{
		return;
	}

	InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UEDItemCraftingWidget::HandleInventoryChanged);
}

void UEDItemCraftingWidget::RefreshCraftRecipes()
{
	CachedRecipeEntries.Reset();

	if (!InventoryComponent)
	{
		DisplayedRecipeRowId = NAME_None;
		RebuildRecipeEntries();
		RebuildCraftTreeNodes();
		RefreshSelectedRecipeSummary();
		return;
	}

	TArray<FEDCraftableRecipeEntry> AllRecipeEntries;
	GatherAllRecipeEntries(AllRecipeEntries);

	for (const FEDCraftableRecipeEntry& RecipeEntry : AllRecipeEntries)
	{
		if (IsRecipeInSelectedCategory(RecipeEntry))
		{
			CachedRecipeEntries.Add(RecipeEntry);
		}
	}

	// 기본 카테고리나 이전 카테고리에 해당하는 레시피가 하나도 없으면
	// 현재 전체 레시피 목록의 첫 번째 아이템 카테고리로 자동 이동해서 빈 화면을 피한다.
	if (CachedRecipeEntries.Num() <= 0 && AllRecipeEntries.Num() > 0)
	{
		const EEDEquippableType FallbackCategory = ResolveRecipeCategory(AllRecipeEntries[0]);
		if (FallbackCategory != EEDEquippableType::None && FallbackCategory != SelectedCraftCategory)
		{
			SelectedCraftCategory = FallbackCategory;

			for (const FEDCraftableRecipeEntry& RecipeEntry : AllRecipeEntries)
			{
				if (IsRecipeInSelectedCategory(RecipeEntry))
				{
					CachedRecipeEntries.Add(RecipeEntry);
				}
			}
		}
	}

	// 현재 표시 대상으로 잡고 있는 레시피가 사라졌으면 목록의 첫 번째 레시피로 보정
	const bool bHasDisplayedRecipe = CachedRecipeEntries.ContainsByPredicate([this](const FEDCraftableRecipeEntry& Entry)
	{
		return Entry.RowId == DisplayedRecipeRowId;
	});

	if (!bHasDisplayedRecipe)
	{
		DisplayedRecipeRowId = CachedRecipeEntries.Num() > 0 ? CachedRecipeEntries[0].RowId : NAME_None;
	}

	RebuildRecipeEntries();
	RebuildCraftTreeNodes();
	RefreshSelectedRecipeSummary();
}

void UEDItemCraftingWidget::GatherAllRecipeEntries(TArray<FEDCraftableRecipeEntry>& OutRecipeEntries) const
{
	OutRecipeEntries.Reset();

	if (!InventoryComponent)
	{
		return;
	}

	TArray<UDataTable*> RecipeTables;
	InventoryComponent->GetAllCraftingRecipeTables(RecipeTables);

	for (UDataTable* RecipeTable : RecipeTables)
	{
		if (!RecipeTable)
		{
			continue;
		}

		const TArray<FName> RowNames = RecipeTable->GetRowNames();
		for (const FName RowName : RowNames)
		{
			const FEDCraftingRecipeRow* RecipeRow = RecipeTable->FindRow<FEDCraftingRecipeRow>(RowName, TEXT("GatherAllRecipeEntries"));
			if (!RecipeRow || !RecipeRow->ResultItemId.IsValid() || RecipeRow->ResultQuantity <= 0)
			{
				continue;
			}

			FEDCraftableRecipeEntry Entry;
			Entry.RowId = RowName;
			Entry.RecipeId = RecipeRow->RecipeId;
			Entry.ResultItemId = RecipeRow->ResultItemId;

			const UEDInventoryItemDataAsset* ResultData = ResolveCraftItemData(RecipeRow->ResultItemId);
			Entry.ResultItemName = ResultData && !ResultData->DisplayName.IsEmpty()
				? ResultData->DisplayName
				: FText::FromName(RecipeRow->ResultItemId.PrimaryAssetName);
			Entry.ResultRarity = ResultData ? ResultData->Rarity : EEDItemRarity::Normal;

			OutRecipeEntries.Add(MoveTemp(Entry));
		}
	}

	OutRecipeEntries.Sort([](const FEDCraftableRecipeEntry& A, const FEDCraftableRecipeEntry& B)
	{
		if (A.ResultRarity != B.ResultRarity)
		{
			return static_cast<uint8>(A.ResultRarity) > static_cast<uint8>(B.ResultRarity);
		}

		return FCString::Stricmp(*A.ResultItemName.ToString(), *B.ResultItemName.ToString()) < 0;
	});
}

bool UEDItemCraftingWidget::IsRecipeInSelectedCategory(const FEDCraftableRecipeEntry& InRecipeData) const
{
	if (SelectedCraftCategory == EEDEquippableType::None)
	{
		return true;
	}

	const UEDInventoryItemDataAsset* ResultData = ResolveCraftItemData(InRecipeData.ResultItemId);
	if (!ResultData || ResultData->ItemType != EEDInventoryItemType::Equippable)
	{
		return false;
	}

	return ResultData->EquippableType == SelectedCraftCategory;
}

EEDEquippableType UEDItemCraftingWidget::ResolveRecipeCategory(const FEDCraftableRecipeEntry& InRecipeData) const
{
	const UEDInventoryItemDataAsset* ResultData = ResolveCraftItemData(InRecipeData.ResultItemId);
	if (!ResultData || ResultData->ItemType != EEDInventoryItemType::Equippable)
	{
		return EEDEquippableType::None;
	}

	return ResultData->EquippableType;
}

void UEDItemCraftingWidget::RebuildRecipeEntries()
{
	if (!RecipeListContainer)
	{
		return;
	}

	RecipeListContainer->ClearChildren();
	RecipeEntryWidgets.Reset();

	if (!RecipeEntryWidgetClass)
	{
		return;
	}

	for (const FEDCraftableRecipeEntry& RecipeEntry : CachedRecipeEntries)
	{
		UTexture2D* ResultIconTexture = nullptr;
		FPrimaryAssetId IgnoredResultItemId;
		ResolveRecipeDisplayData(RecipeEntry, ResultIconTexture, IgnoredResultItemId);

		UEDCraftRecipeEntryWidget* EntryWidget = CreateWidget<UEDCraftRecipeEntryWidget>(this, RecipeEntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		FEDCraftRecipeEntryDisplayData DisplayData;
		DisplayData.RowId = RecipeEntry.RowId;
		DisplayData.ResultItemName = RecipeEntry.ResultItemName;
		DisplayData.ResultRarity = RecipeEntry.ResultRarity;
		DisplayData.ResultIconTexture = ResultIconTexture;

		EntryWidget->SetRecipeEntryData(DisplayData);
		EntryWidget->SetSelectedState(RecipeEntry.RowId == DisplayedRecipeRowId);
		EntryWidget->OnRecipeEntryClicked.AddUObject(this, &UEDItemCraftingWidget::HandleRecipeEntryClicked);

		RecipeListContainer->AddChild(EntryWidget);
		RecipeEntryWidgets.Add(EntryWidget);
	}
}

void UEDItemCraftingWidget::RebuildCraftTreeNodes()
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

	if (!CraftTreeNodeWidgetClass)
	{
		return;
	}

	FEDCraftableRecipeEntry CraftTargetRecipe;
	if (!TryGetDisplayedRecipeEntry(CraftTargetRecipe))
	{
		return;
	}

	bool bCyclePruned = false;
	UEDInventoryBlueprintLibrary::BuildCraftTreeFlat(
		InventoryComponent,
		CraftTargetRecipe.ResultItemId,
		CachedFlatTreeNodes,
		bCyclePruned,
		8);

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

		const UEDInventoryItemDataAsset* ItemData = ResolveCraftItemData(FlatNode.ItemId);
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

void UEDItemCraftingWidget::RebuildCraftTreeLines()
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

		// 각 노드 위젯 안에서 "부모는 아래 중앙", "자식은 위 중앙" 지점을 연결 시작점으로 설정
		// Geometry는 각 위젯 자신의 좌표계를 쓰므로, 먼저 절대 좌표로 변환
		const FVector2D ParentBottomAbsolute = ParentGeometry.LocalToAbsolute(
			FVector2D(ParentGeometry.GetLocalSize().X * 0.5f, ParentGeometry.GetLocalSize().Y - ParentBottomInset));
		const FVector2D ChildTopAbsolute = ChildGeometry.LocalToAbsolute(
			FVector2D(ChildGeometry.GetLocalSize().X * 0.5f, ChildTopInset));

		// 선은 CraftTreeLineCanvas 위에 배치해야 하므로,
		// 절대 좌표를 다시 "라인 캔버스 기준 로컬 좌표"로 변환
		const FVector2D ParentBottomLocal = LineCanvasGeometry.AbsoluteToLocal(ParentBottomAbsolute);
		const FVector2D ChildTopLocal = LineCanvasGeometry.AbsoluteToLocal(ChildTopAbsolute);
		if (ChildTopLocal.Y <= ParentBottomLocal.Y)
		{
			continue;
		}

		// MidY - 가로선이 지나갈 높이
		float MidY = FMath::Min(
			ParentBottomLocal.Y + ParentStemLength,
			ChildTopLocal.Y - ChildStemLength);

		// 두 노드 간 간격이 너무 좁으면
		// 부모-자식 사이의 중간 높이를 써서 선이 뒤집히지 않게 막음
		if (MidY <= ParentBottomLocal.Y || MidY >= ChildTopLocal.Y)
		{
			MidY = ParentBottomLocal.Y + ((ChildTopLocal.Y - ParentBottomLocal.Y) * 0.5f);
		}

		AddCraftTreeLineSegment(ParentBottomLocal, FVector2D(ParentBottomLocal.X, MidY), LineColor, LineThickness);
		AddCraftTreeLineSegment(FVector2D(ParentBottomLocal.X, MidY), FVector2D(ChildTopLocal.X, MidY), LineColor,
		                        LineThickness);
		AddCraftTreeLineSegment(FVector2D(ChildTopLocal.X, MidY), ChildTopLocal, LineColor, LineThickness);
	}
}

void UEDItemCraftingWidget::AddCraftTreeLineSegment(
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

	// 선 위젯은 얇은 사각형 Border를 가로/세로로 배치해서 만듦
	// 구간이 가로선인지 세로선인지 먼저 판단한 뒤, 캔버스에 올릴 위치와 크기를 계산
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

void UEDItemCraftingWidget::MarkCraftTreeLinesDirty()
{
	bCraftTreeLinesDirty = true;
	LastCraftTreeLineCanvasSize = FVector2D::ZeroVector;
	LastCraftTreeContentSize = FVector2D::ZeroVector;
	LastCraftTreeLayoutHash = 0;
}

uint32 UEDItemCraftingWidget::BuildCraftTreeLayoutHash() const
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
	// 창 크기는 그대로인데 정렬 결과만 달라져도 선을 다시 만들어야 하므로,
	// 각 노드의 중심 좌표와 크기를 함께 해시에 넣음
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

		// 실수 좌표를 그대로 비교하면 미세한 흔들림에도 너무 자주 다시 그릴 수 있어서
		// 적당히 반올림한 값으로만 비교
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(NodeId));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeCenterLocal.X * 10.0f)));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeCenterLocal.Y * 10.0f)));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeSize.X * 10.0f)));
		LayoutHash = HashCombineFast(LayoutHash, GetTypeHash(FMath::RoundToInt(NodeSize.Y * 10.0f)));
	}

	return LayoutHash;
}

void UEDItemCraftingWidget::RefreshSelectedRecipeSummary()
{
	FEDCraftableRecipeEntry CraftTargetRecipe;
	if (!TryGetDisplayedRecipeEntry(CraftTargetRecipe))
	{
		if (SelectedRecipeIconImage)
		{
			SelectedRecipeIconImage->SetBrushFromTexture(nullptr);
		}

		if (SelectedRecipeNameText)
		{
			SelectedRecipeNameText->SetText(FText::FromString(TEXT("선택된 레시피가 없습니다.")));
		}

		if (SelectedRecipeStateText)
		{
			SelectedRecipeStateText->SetText(FText::GetEmpty());
		}

		return;
	}

	UTexture2D* ResultIconTexture = nullptr;
	FPrimaryAssetId ResultItemId;
	ResolveRecipeDisplayData(CraftTargetRecipe, ResultIconTexture, ResultItemId);

	if (SelectedRecipeIconImage)
	{
		SelectedRecipeIconImage->SetBrushFromTexture(ResultIconTexture);
	}

	if (SelectedRecipeNameText)
	{
		SelectedRecipeNameText->SetText(CraftTargetRecipe.ResultItemName);
	}

	if (SelectedRecipeStateText)
	{
		SelectedRecipeStateText->SetText(FText::FromString(TEXT("제작 키를 눌러 아이템을 제작할 수 있습니다.")));
	}
}

bool UEDItemCraftingWidget::TryGetDisplayedRecipeEntry(FEDCraftableRecipeEntry& OutRecipeData) const
{
	if (CachedRecipeEntries.Num() <= 0)
	{
		return false;
	}

	for (const FEDCraftableRecipeEntry& RecipeEntry : CachedRecipeEntries)
	{
		if (RecipeEntry.RowId == DisplayedRecipeRowId)
		{
			OutRecipeData = RecipeEntry;
			return true;
		}
	}

	OutRecipeData = CachedRecipeEntries[0];
	return true;
}

bool UEDItemCraftingWidget::TryGetFirstCraftableRecipeEntry(FEDCraftableRecipeEntry& OutRecipeData) const
{
	if (!InventoryComponent)
	{
		return false;
	}

	TArray<FEDCraftableRecipeEntry> CraftableRecipeEntries;
	InventoryComponent->GetCachedCraftableRecipes(CraftableRecipeEntries);
	if (CraftableRecipeEntries.Num() <= 0)
	{
		return false;
	}

	OutRecipeData = CraftableRecipeEntries[0];
	return true;
}

bool UEDItemCraftingWidget::ResolveRecipeDisplayData(
	const FEDCraftableRecipeEntry& InRecipeData,
	UTexture2D*& OutIconTexture,
	FPrimaryAssetId& OutResultItemId) const
{
	OutIconTexture = nullptr;
	OutResultItemId = InRecipeData.ResultItemId;

	const UEDInventoryItemDataAsset* ResultData = ResolveCraftItemData(InRecipeData.ResultItemId);
	if (!ResultData)
	{
		return false;
	}

	OutIconTexture = ResultData->IconTexture;
	return true;
}

void UEDItemCraftingWidget::HandleWeaponCategoryClicked()
{
	SelectedCraftCategory = EEDEquippableType::Weapon;
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::HandleTopArmorCategoryClicked()
{
	SelectedCraftCategory = EEDEquippableType::TopArmor;
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::HandleBottomArmorCategoryClicked()
{
	SelectedCraftCategory = EEDEquippableType::BottomArmor;
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::HandleInventoryChanged()
{
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::HandleRecipeEntryClicked(FName InRecipeRowId)
{
	if (InRecipeRowId.IsNone() || DisplayedRecipeRowId == InRecipeRowId)
	{
		return;
	}

	DisplayedRecipeRowId = InRecipeRowId;
	RebuildRecipeEntries();
	RebuildCraftTreeNodes();
	RefreshSelectedRecipeSummary();
}
