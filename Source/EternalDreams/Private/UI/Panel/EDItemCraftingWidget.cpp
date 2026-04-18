// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDItemCraftingWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDItemDataRows.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/Message/EDUserFacingMessage.h"
#include "UI/HUD/EDCraftRecipeEntryWidget.h"
#include "UI/Panel/EDCraftTreeWidget.h"
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
	return EDUserFacingMessage::Craft::GetFailureText(Failure);
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
				UIManageSubsystem->ShowToastMessage(EDUserFacingMessage::Craft::GetFailureText(EEDInventoryActionFailure::InvalidInventory), EEDUIMessageType::Error, 3.0f);
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
				UIManageSubsystem->ShowToastMessage(EDUserFacingMessage::Craft::GetFailureText(EEDInventoryActionFailure::InvalidRecipe), EEDUIMessageType::Error, 3.0f);
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
				UIManageSubsystem->ShowToastMessage(EDUserFacingMessage::Craft::GetFailureText(Failure), EEDUIMessageType::Error, 3.0f);
			}
		}
		return false;
	}

	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
		{
			UIManageSubsystem->ShowToastMessage(
				EDUserFacingMessage::Craft::GetSuccessText(CraftTargetRecipe.ResultItemName),
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
	CachedRecipeEntries.Reset();

	if (!InventoryComponent)
	{
		DisplayedRecipeRowId = NAME_None;
		RebuildRecipeEntries();
		RefreshSelectedRecipeSummary();
		if (CraftTreeWidget)
		{
			CraftTreeWidget->SetInventoryComponent(nullptr);
			CraftTreeWidget->SetDisplayedResultItemId(FPrimaryAssetId());
			CraftTreeWidget->RefreshTree();
		}
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

	const bool bHasDisplayedRecipe = CachedRecipeEntries.ContainsByPredicate([this](const FEDCraftableRecipeEntry& Entry)
	{
		return Entry.RowId == DisplayedRecipeRowId;
	});

	if (!bHasDisplayedRecipe)
	{
		DisplayedRecipeRowId = CachedRecipeEntries.Num() > 0 ? CachedRecipeEntries[0].RowId : NAME_None;
	}

	RebuildRecipeEntries();
	RefreshSelectedRecipeSummary();

	FEDCraftableRecipeEntry DisplayedRecipe;
	if (CraftTreeWidget)
	{
		CraftTreeWidget->SetInventoryComponent(InventoryComponent);
		CraftTreeWidget->SetDisplayedResultItemId(
			TryGetDisplayedRecipeEntry(DisplayedRecipe) ? DisplayedRecipe.ResultItemId : FPrimaryAssetId());
		CraftTreeWidget->RefreshTree();
	}
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

void UEDItemCraftingWidget::RefreshSelectedRecipeSummary()
{
	FEDCraftableRecipeEntry DisplayedRecipe;
	if (!TryGetDisplayedRecipeEntry(DisplayedRecipe))
	{
		if (SelectedRecipeIconImage)
		{
			SelectedRecipeIconImage->SetBrushFromTexture(nullptr);
		}

		if (SelectedRecipeNameText)
		{
			SelectedRecipeNameText->SetText(FText::FromString(TEXT("선택된 레시피가 없습니다.")));
		}

		return;
	}

	UTexture2D* ResultIconTexture = nullptr;
	FPrimaryAssetId ResultItemId;
	ResolveRecipeDisplayData(DisplayedRecipe, ResultIconTexture, ResultItemId);

	if (SelectedRecipeIconImage)
	{
		SelectedRecipeIconImage->SetBrushFromTexture(ResultIconTexture);
	}

	if (SelectedRecipeNameText)
	{
		SelectedRecipeNameText->SetText(DisplayedRecipe.ResultItemName);
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
	RefreshSelectedRecipeSummary();

	FEDCraftableRecipeEntry DisplayedRecipe;
	if (CraftTreeWidget)
	{
		CraftTreeWidget->SetDisplayedResultItemId(
			TryGetDisplayedRecipeEntry(DisplayedRecipe) ? DisplayedRecipe.ResultItemId : FPrimaryAssetId());
		CraftTreeWidget->RefreshTree();
	}
}



