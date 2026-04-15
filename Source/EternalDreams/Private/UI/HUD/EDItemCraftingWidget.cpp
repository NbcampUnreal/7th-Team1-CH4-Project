// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDItemCraftingWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
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
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Rendering/DrawElements.h"
#include "UI/HUD/EDCraftRecipeEntryWidget.h"
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
		return FText::FromString(TEXT("더 이상 같은 아이템을 쌓을 수 없습니다."));
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
		BottomArmorCategoryButton->OnClicked.RemoveDynamic(this, &UEDItemCraftingWidget::HandleBottomArmorCategoryClicked);
	}

	Super::NativeDestruct();
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
	int32 PaintLayerId = LayerId;

	if (CraftTreeContainer && CachedFlatTreeNodes.Num() > 0 && CraftTreeNodeWidgetMap.Num() > 0)
	{
		const FLinearColor LineColor(0.42f, 0.36f, 0.29f, 1.0f);
		const float LineThickness = 2.0f;
		const float ParentBottomInset = 3.0f;
		const float ChildTopInset = 3.0f;

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

			const FVector2D ParentBottomAbsolute = ParentGeometry.LocalToAbsolute(
				FVector2D(ParentGeometry.GetLocalSize().X * 0.5f, ParentGeometry.GetLocalSize().Y - ParentBottomInset));
			const FVector2D ChildTopAbsolute = ChildGeometry.LocalToAbsolute(
				FVector2D(ChildGeometry.GetLocalSize().X * 0.5f, ChildTopInset));

			const FVector2D ParentBottomLocal = AllottedGeometry.AbsoluteToLocal(ParentBottomAbsolute);
			const FVector2D ChildTopLocal = AllottedGeometry.AbsoluteToLocal(ChildTopAbsolute);
			if (ChildTopLocal.Y <= ParentBottomLocal.Y)
			{
				continue;
			}

			const float MidY = ParentBottomLocal.Y + ((ChildTopLocal.Y - ParentBottomLocal.Y) * 0.5f);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				PaintLayerId,
				AllottedGeometry.ToPaintGeometry(),
				{ParentBottomLocal, FVector2D(ParentBottomLocal.X, MidY)},
				ESlateDrawEffect::None,
				LineColor,
				true,
				LineThickness);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				PaintLayerId,
				AllottedGeometry.ToPaintGeometry(),
				{FVector2D(ParentBottomLocal.X, MidY), FVector2D(ChildTopLocal.X, MidY)},
				ESlateDrawEffect::None,
				LineColor,
				true,
				LineThickness);

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				PaintLayerId,
				AllottedGeometry.ToPaintGeometry(),
				{FVector2D(ChildTopLocal.X, MidY), ChildTopLocal},
				ESlateDrawEffect::None,
				LineColor,
				true,
				LineThickness);
		}

		++PaintLayerId;
	}

	return Super::NativePaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		PaintLayerId,
		InWidgetStyle,
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
		ShowCraftFailure(EEDInventoryActionFailure::InvalidInventory);
		return false;
	}

	if (SelectedRecipeRowId.IsNone())
	{
		ShowCraftFailure(EEDInventoryActionFailure::InvalidRecipe);
		return false;
	}

	FEDCraftRecipeViewData SelectedRecipe;
	if (!TryGetSelectedRecipeViewData(SelectedRecipe))
	{
		ShowCraftFailure(EEDInventoryActionFailure::InvalidRecipe);
		return false;
	}

	if (!SelectedRecipe.bCanCraft)
	{
		bool bHasMissingIngredient = false;
		for (const FEDCraftIngredientViewData& Ingredient : SelectedRecipe.Ingredients)
		{
			if (!Ingredient.bSatisfied)
			{
				bHasMissingIngredient = true;
				break;
			}
		}

		ShowCraftFailure(bHasMissingIngredient
			? EEDInventoryActionFailure::MissingIngredient
			: EEDInventoryActionFailure::NoSpace);
		return false;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSuccess = InventoryComponent->RequestCraftItemDetailed(SelectedRecipeRowId, Failure);
	if (!bSuccess)
	{
		ShowCraftFailure(Failure);
		return false;
	}

	ShowCraftSuccess(FText::Format(
		FText::FromString(TEXT("{0} 제작에 성공했습니다.")),
		SelectedRecipe.ResultItemName));
	return true;
}

void UEDItemCraftingWidget::InitializeInventoryComponent()
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: OwningPlayerPawn을 찾을 수 없습니다."));
		return;
	}

	InventoryComponent = UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(OwningPawn);
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: InventoryComponent를 찾을 수 없습니다."));
	}
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
		BottomArmorCategoryButton->OnClicked.RemoveDynamic(this, &UEDItemCraftingWidget::HandleBottomArmorCategoryClicked);
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
	CachedRecipeViews.Reset();

	if (!InventoryComponent)
	{
		RebuildRecipeEntries();
		RebuildCraftTreeNodes();
		RefreshSelectedRecipeSummary();
		return;
	}

	TArray<FEDCraftRecipeViewData> AllRecipeViews;
	UEDInventoryBlueprintLibrary::GetCraftRecipeViewDataList(
		InventoryComponent,
		AllRecipeViews,
		false,
		EEDCraftableRecipeSortOption::ByRarity,
		true);

	for (const FEDCraftRecipeViewData& RecipeView : AllRecipeViews)
	{
		if (IsRecipeInSelectedCategory(RecipeView))
		{
			CachedRecipeViews.Add(RecipeView);
		}
	}

	if (SelectedRecipeRowId.IsNone() && CachedRecipeViews.Num() > 0)
	{
		SelectedRecipeRowId = CachedRecipeViews[0].RowId;
	}

	bool bHasSelectedRecipe = false;
	for (const FEDCraftRecipeViewData& RecipeView : CachedRecipeViews)
	{
		if (RecipeView.RowId == SelectedRecipeRowId)
		{
			bHasSelectedRecipe = true;
			break;
		}
	}

	if (!bHasSelectedRecipe)
	{
		SelectedRecipeRowId = CachedRecipeViews.Num() > 0 ? CachedRecipeViews[0].RowId : NAME_None;
	}

	RebuildRecipeEntries();
	RebuildCraftTreeNodes();
	RefreshSelectedRecipeSummary();
}

bool UEDItemCraftingWidget::IsRecipeInSelectedCategory(const FEDCraftRecipeViewData& InRecipeData) const
{
	if (SelectedCraftCategory == EEDEquippableType::None)
	{
		return true;
	}

	const UEDInventoryItemDataAsset* ResultData = ResolveCraftTreeItemData(InRecipeData.ResultItemId);
	if (!ResultData || ResultData->ItemType != EEDInventoryItemType::Equippable)
	{
		return false;
	}

	return ResultData->EquippableType == SelectedCraftCategory;
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
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: RecipeEntryWidgetClass가 설정되지 않았습니다."));
		return;
	}

	for (const FEDCraftRecipeViewData& RecipeView : CachedRecipeViews)
	{
		UEDCraftRecipeEntryWidget* EntryWidget = CreateWidget<UEDCraftRecipeEntryWidget>(this, RecipeEntryWidgetClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->SetRecipeViewData(RecipeView);
		EntryWidget->SetSelectedState(RecipeView.RowId == SelectedRecipeRowId);
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

	if (!CraftTreeNodeWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDItemCraftingWidget: CraftTreeNodeWidgetClass가 설정되지 않았습니다."));
		return;
	}

	FEDCraftRecipeViewData SelectedRecipe;
	if (!TryGetSelectedRecipeViewData(SelectedRecipe))
	{
		return;
	}

	bool bCyclePruned = false;
	UEDInventoryBlueprintLibrary::BuildCraftTreeFlat(
		InventoryComponent,
		SelectedRecipe.ResultItemId,
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

		FEDCraftTreeNodeViewData NodeViewData;
		if (!BuildCraftTreeNodeViewData(FlatNode, NodeViewData))
		{
			continue;
		}

		UEDCraftTreeNodeWidget* NodeWidget = CreateWidget<UEDCraftTreeNodeWidget>(this, CraftTreeNodeWidgetClass);
		if (!NodeWidget)
		{
			continue;
		}

		NodeWidget->SetTreeNodeViewData(NodeViewData);
		if (UHorizontalBoxSlot* NodeSlot = CurrentRow->AddChildToHorizontalBox(NodeWidget))
		{
			NodeSlot->SetPadding(TreeNodePadding);
			NodeSlot->SetHorizontalAlignment(HAlign_Center);
			NodeSlot->SetVerticalAlignment(VAlign_Center);
		}
		CraftTreeNodeWidgets.Add(NodeWidget);
		CraftTreeNodeWidgetMap.Add(FlatNode.NodeId, NodeWidget);
	}
}

void UEDItemCraftingWidget::RefreshSelectedRecipeSummary()
{
	FEDCraftRecipeViewData SelectedRecipe;
	if (!TryGetSelectedRecipeViewData(SelectedRecipe))
	{
		if (SelectedRecipeIconImage)
		{
			SelectedRecipeIconImage->SetBrushFromTexture(nullptr);
		}

		if (SelectedRecipeNameText)
		{
			SelectedRecipeNameText->SetText(FText::FromString(TEXT("선택된 제작식이 없습니다.")));
		}

		if (SelectedRecipeStateText)
		{
			SelectedRecipeStateText->SetText(FText::GetEmpty());
		}

		return;
	}

	if (SelectedRecipeIconImage)
	{
		SelectedRecipeIconImage->SetBrushFromTexture(SelectedRecipe.ResultIconTexture);
	}

	if (SelectedRecipeNameText)
	{
		SelectedRecipeNameText->SetText(SelectedRecipe.ResultItemName);
	}

	if (SelectedRecipeStateText)
	{
		SelectedRecipeStateText->SetText(
			SelectedRecipe.bCanCraft
				? FText::FromString(TEXT("제작 키를 눌러 아이템을 제작할 수 있습니다."))
				: FText::FromString(TEXT("필요한 재료를 모두 모아야 제작할 수 있습니다.")));
	}
}

bool UEDItemCraftingWidget::TryGetSelectedRecipeViewData(FEDCraftRecipeViewData& OutRecipeData) const
{
	for (const FEDCraftRecipeViewData& RecipeView : CachedRecipeViews)
	{
		if (RecipeView.RowId == SelectedRecipeRowId)
		{
			OutRecipeData = RecipeView;
			return true;
		}
	}

	return false;
}

bool UEDItemCraftingWidget::BuildCraftTreeNodeViewData(const FEDCraftTreeFlatNode& InFlatNode, FEDCraftTreeNodeViewData& OutNodeData) const
{
	OutNodeData = FEDCraftTreeNodeViewData();

	if (!InFlatNode.ItemId.IsValid())
	{
		return false;
	}

	const UEDInventoryItemDataAsset* ItemData = ResolveCraftTreeItemData(InFlatNode.ItemId);

	OutNodeData.NodeId = InFlatNode.NodeId;
	OutNodeData.ParentNodeId = InFlatNode.ParentNodeId;
	OutNodeData.Depth = InFlatNode.Depth;
	OutNodeData.ItemId = InFlatNode.ItemId;
	OutNodeData.DisplayName = ItemData && !ItemData->DisplayName.IsEmpty()
		? ItemData->DisplayName
		: FText::FromName(InFlatNode.ItemId.PrimaryAssetName);
	OutNodeData.Rarity = ItemData ? ItemData->Rarity : EEDItemRarity::Normal;
	OutNodeData.IconTexture = ItemData ? ItemData->IconTexture : nullptr;
	OutNodeData.RequiredQuantity = FMath::Max(1, InFlatNode.Quantity);
	OutNodeData.OwnedQuantity = CountOwnedItemQuantity(InFlatNode.ItemId);
	OutNodeData.bSatisfied = OutNodeData.OwnedQuantity >= OutNodeData.RequiredQuantity;
	OutNodeData.bIsCraftable = InFlatNode.bIsCraftable;
	return true;
}

int32 UEDItemCraftingWidget::CountOwnedItemQuantity(const FPrimaryAssetId& ItemId) const
{
	if (!InventoryComponent || !ItemId.IsValid())
	{
		return 0;
	}

	int32 OwnedQuantity = 0;

	for (const FEDInventorySlotData& SlotData : InventoryComponent->InventorySlots)
	{
		if (!SlotData.IsEmpty() && SlotData.Item.ItemId == ItemId)
		{
			OwnedQuantity += SlotData.Item.Quantity;
		}
	}

	const TArray<const FEDEquipmentSlotData*> EquipmentSlots =
	{
		&InventoryComponent->WeaponSlot,
		&InventoryComponent->TopArmorSlot,
		&InventoryComponent->BottomArmorSlot
	};

	for (const FEDEquipmentSlotData* EquipmentSlot : EquipmentSlots)
	{
		if (EquipmentSlot && EquipmentSlot->EquippedItem.IsValid() && EquipmentSlot->EquippedItem.ItemId == ItemId)
		{
			OwnedQuantity += EquipmentSlot->EquippedItem.Quantity;
		}
	}

	return OwnedQuantity;
}

void UEDItemCraftingWidget::HandleRecipeEntryClicked(FName InRecipeRowId)
{
	if (InRecipeRowId.IsNone())
	{
		return;
	}

	SelectedRecipeRowId = InRecipeRowId;
	RebuildRecipeEntries();
	RebuildCraftTreeNodes();
	RefreshSelectedRecipeSummary();
}

void UEDItemCraftingWidget::HandleWeaponCategoryClicked()
{
	SelectedCraftCategory = EEDEquippableType::Weapon;
	SelectedRecipeRowId = NAME_None;
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::HandleTopArmorCategoryClicked()
{
	SelectedCraftCategory = EEDEquippableType::TopArmor;
	SelectedRecipeRowId = NAME_None;
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::HandleBottomArmorCategoryClicked()
{
	SelectedCraftCategory = EEDEquippableType::BottomArmor;
	SelectedRecipeRowId = NAME_None;
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::HandleInventoryChanged()
{
	RefreshCraftRecipes();
}

void UEDItemCraftingWidget::ShowCraftSuccess(const FText& InMessage) const
{
	if (!ActionResultText)
	{
		return;
	}

	ActionResultText->SetColorAndOpacity(FSlateColor(FLinearColor(0.19f, 0.62f, 0.27f, 1.0f)));
	ActionResultText->SetText(InMessage);
	ActionResultText->SetVisibility(ESlateVisibility::Visible);
}

void UEDItemCraftingWidget::ShowCraftFailure(EEDInventoryActionFailure Failure) const
{
	if (!ActionResultText)
	{
		return;
	}

	ActionResultText->SetColorAndOpacity(FSlateColor(FLinearColor(0.77f, 0.27f, 0.21f, 1.0f)));
	ActionResultText->SetText(GetCraftFailureText(Failure));
	ActionResultText->SetVisibility(ESlateVisibility::Visible);
}

void UEDItemCraftingWidget::ClearActionMessage() const
{
	if (!ActionResultText)
	{
		return;
	}

	ActionResultText->SetText(FText::GetEmpty());
	ActionResultText->SetVisibility(ESlateVisibility::Collapsed);
}
