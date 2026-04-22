// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDSkillBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Characters/Player/Component/SkillComponent.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Engine/AssetManager.h"
#include "Engine/LocalPlayer.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "UI/HUD/EDSkillSlotWidget.h"
#include "UI/Message/EDUserFacingMessage.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"

namespace
{
const UEDInventoryItemDataAsset* ResolveSkillBarItemData(const FPrimaryAssetId& ItemId)
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

void UEDSkillBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (QSkillSlot)
	{
		QSkillSlot->SetTargetSkillSlotType(EEDSkillSlotType::FirstSkill);
		QSkillSlot->OnSkillSlotDropped.AddUObject(this, &UEDSkillBarWidget::HandleFirstSkillSlotDropped);
		QSkillSlot->OnSkillSlotDoubleClicked.AddUObject(this, &UEDSkillBarWidget::HandleFirstSkillSlotDoubleClicked);
	}

	if (ESkillSlot)
	{
		ESkillSlot->SetTargetSkillSlotType(EEDSkillSlotType::SecondSkill);
		ESkillSlot->OnSkillSlotDropped.AddUObject(this, &UEDSkillBarWidget::HandleSecondSkillSlotDropped);
		ESkillSlot->OnSkillSlotDoubleClicked.AddUObject(this, &UEDSkillBarWidget::HandleSecondSkillSlotDoubleClicked);
	}

	InitializeReferences();
	BindInventoryDelegates();
	BindSkillDelegates();
	RequestDeferredRefresh();
}

void UEDSkillBarWidget::NativeDestruct()
{
	UnbindSkillDelegates();
	UnbindInventoryDelegates();

	Super::NativeDestruct();
}

void UEDSkillBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshReferencesIfNeeded();

	if (bDeferredRefreshRequested)
	{
		RefreshSkillBar();
		bDeferredRefreshRequested = false;
	}
}

void UEDSkillBarWidget::InitializeReferences()
{
	CachedPlayerCharacter = Cast<AEDPlayerCharacter>(GetOwningPlayerPawn());
	CachedInventoryComponent = CachedPlayerCharacter ? UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(CachedPlayerCharacter) : nullptr;
	CachedSkillComponent = CachedPlayerCharacter ? CachedPlayerCharacter->GetSkillComponent() : nullptr;
	CachedAbilitySystemComponent = CachedPlayerCharacter ? CachedPlayerCharacter->GetAbilitySystemComponent() : nullptr;
}

void UEDSkillBarWidget::RefreshReferencesIfNeeded()
{
	AEDPlayerCharacter* CurrentPlayerCharacter = Cast<AEDPlayerCharacter>(GetOwningPlayerPawn());
	UEDInventoryComponent* CurrentInventoryComponent = CurrentPlayerCharacter ? UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(CurrentPlayerCharacter) : nullptr;
	USkillComponent* CurrentSkillComponent = CurrentPlayerCharacter ? CurrentPlayerCharacter->GetSkillComponent() : nullptr;
	UAbilitySystemComponent* CurrentAbilitySystemComponent = CurrentPlayerCharacter ? CurrentPlayerCharacter->GetAbilitySystemComponent() : nullptr;

	if (CachedPlayerCharacter == CurrentPlayerCharacter &&
		CachedInventoryComponent == CurrentInventoryComponent &&
		CachedSkillComponent == CurrentSkillComponent &&
		CachedAbilitySystemComponent == CurrentAbilitySystemComponent)
	{
		return;
	}

	UnbindSkillDelegates();
	UnbindInventoryDelegates();

	InitializeReferences();

	BindInventoryDelegates();
	BindSkillDelegates();
	RequestDeferredRefresh();
}

void UEDSkillBarWidget::BindInventoryDelegates()
{
	if (!CachedInventoryComponent)
	{
		return;
	}

	CachedInventoryComponent->OnWeaponSlotChanged.AddUniqueDynamic(this, &UEDSkillBarWidget::HandleWeaponSlotChanged);
	CachedInventoryComponent->OnFirstSkillSlotChanged.AddUniqueDynamic(this, &UEDSkillBarWidget::HandleFirstSkillSlotChanged);
	CachedInventoryComponent->OnSecondSkillSlotChanged.AddUniqueDynamic(this, &UEDSkillBarWidget::HandleSecondSkillSlotChanged);
}

void UEDSkillBarWidget::UnbindInventoryDelegates()
{
	if (!CachedInventoryComponent)
	{
		return;
	}

	CachedInventoryComponent->OnWeaponSlotChanged.RemoveDynamic(this, &UEDSkillBarWidget::HandleWeaponSlotChanged);
	CachedInventoryComponent->OnFirstSkillSlotChanged.RemoveDynamic(this, &UEDSkillBarWidget::HandleFirstSkillSlotChanged);
	CachedInventoryComponent->OnSecondSkillSlotChanged.RemoveDynamic(this, &UEDSkillBarWidget::HandleSecondSkillSlotChanged);
}

void UEDSkillBarWidget::BindSkillDelegates()
{
	if (!CachedSkillComponent)
	{
		return;
	}

	CachedSkillComponent->OnQSkillCoolTime.AddUniqueDynamic(this, &UEDSkillBarWidget::HandleQSkillCoolTime);
	CachedSkillComponent->OnESkillCoolTime.AddUniqueDynamic(this, &UEDSkillBarWidget::HandleESkillCoolTime);
	CachedSkillComponent->OnSpaceSkillCoolTime.AddUniqueDynamic(this, &UEDSkillBarWidget::HandleSpaceSkillCoolTime);
}

void UEDSkillBarWidget::UnbindSkillDelegates()
{
	if (!CachedSkillComponent)
	{
		return;
	}

	CachedSkillComponent->OnQSkillCoolTime.RemoveDynamic(this, &UEDSkillBarWidget::HandleQSkillCoolTime);
	CachedSkillComponent->OnESkillCoolTime.RemoveDynamic(this, &UEDSkillBarWidget::HandleESkillCoolTime);
	CachedSkillComponent->OnSpaceSkillCoolTime.RemoveDynamic(this, &UEDSkillBarWidget::HandleSpaceSkillCoolTime);
}

void UEDSkillBarWidget::RefreshSkillBar()
{
	RefreshSkillSlotDisplay();
	RefreshCooldownState();
}

void UEDSkillBarWidget::RefreshSkillSlotDisplay() const
{
	if (!CachedInventoryComponent || !CachedSkillComponent)
	{
		if (QSkillSlot)
		{
			FEDSkillSlotDisplayData DisplayData;
			DisplayData.KeyLabel = FText::FromString(TEXT("Q"));
			QSkillSlot->SetSlotDisplayData(DisplayData);
		}

		if (ESkillSlot)
		{
			FEDSkillSlotDisplayData DisplayData;
			DisplayData.KeyLabel = FText::FromString(TEXT("E"));
			ESkillSlot->SetSlotDisplayData(DisplayData);
		}

		if (SpaceSkillSlot)
		{
			FEDSkillSlotDisplayData DisplayData;
			DisplayData.KeyLabel = FText::FromString(TEXT("Space"));
			SpaceSkillSlot->SetSlotDisplayData(DisplayData);
		}

		return;
	}

	if (QSkillSlot)
	{
		FEDSkillSlotDisplayData DisplayData;
		DisplayData.KeyLabel = FText::FromString(TEXT("Q"));
		DisplayData.SkillTag = CachedSkillComponent->GetQSkillTag();
		if (CachedInventoryComponent->FirstSkillSlot.EquippedItem.IsValid())
		{
			if (const UEDInventoryItemDataAsset* ItemData = ResolveItemData(CachedInventoryComponent->FirstSkillSlot.EquippedItem.ItemId))
			{
				DisplayData.IconTexture = ItemData->IconTexture;
			}
		}
		QSkillSlot->SetSlotDisplayData(DisplayData);
	}

	if (ESkillSlot)
	{
		FEDSkillSlotDisplayData DisplayData;
		DisplayData.KeyLabel = FText::FromString(TEXT("E"));
		DisplayData.SkillTag = CachedSkillComponent->GetESkillTag();
		if (CachedInventoryComponent->SecondSkillSlot.EquippedItem.IsValid())
		{
			if (const UEDInventoryItemDataAsset* ItemData = ResolveItemData(CachedInventoryComponent->SecondSkillSlot.EquippedItem.ItemId))
			{
				DisplayData.IconTexture = ItemData->IconTexture;
			}
		}
		ESkillSlot->SetSlotDisplayData(DisplayData);
	}

	if (SpaceSkillSlot)
	{
		FEDSkillSlotDisplayData DisplayData;
		DisplayData.KeyLabel = FText::FromString(TEXT("Space"));
		DisplayData.SkillTag = CachedSkillComponent->GetSpaceSkillTag();
		if (CachedInventoryComponent->WeaponSlot.EquippedItem.IsValid())
		{
			if (const UEDInventoryItemDataAsset* ItemData = ResolveItemData(CachedInventoryComponent->WeaponSlot.EquippedItem.ItemId))
			{
				DisplayData.IconTexture = ItemData->IconTexture;
			}
		}
		SpaceSkillSlot->SetSlotDisplayData(DisplayData);
	}
}

void UEDSkillBarWidget::RefreshCooldownState() const
{
	if (!CachedSkillComponent)
	{
		if (QSkillSlot)
		{
			QSkillSlot->ClearCooldown();
		}

		if (ESkillSlot)
		{
			ESkillSlot->ClearCooldown();
		}

		if (SpaceSkillSlot)
		{
			SpaceSkillSlot->ClearCooldown();
		}

		return;
	}

	float RemainingTime = 0.0f;
	float MaxCooldownTime = 0.0f;

	if (QSkillSlot)
	{
		if (ResolveCooldownFromTag(CachedSkillComponent->GetQSkillCoolTimeTag(), RemainingTime, MaxCooldownTime))
		{
			QSkillSlot->StartCooldown(RemainingTime, MaxCooldownTime);
		}
		else
		{
			QSkillSlot->ClearCooldown();
		}
	}

	if (ESkillSlot)
	{
		if (ResolveCooldownFromTag(CachedSkillComponent->GetESkillCoolTimeTag(), RemainingTime, MaxCooldownTime))
		{
			ESkillSlot->StartCooldown(RemainingTime, MaxCooldownTime);
		}
		else
		{
			ESkillSlot->ClearCooldown();
		}
	}

	if (SpaceSkillSlot)
	{
		if (ResolveCooldownFromTag(CachedSkillComponent->GetSpaceSkillCoolTimeTag(), RemainingTime, MaxCooldownTime))
		{
			SpaceSkillSlot->StartCooldown(RemainingTime, MaxCooldownTime);
		}
		else
		{
			SpaceSkillSlot->ClearCooldown();
		}
	}
}

const UEDInventoryItemDataAsset* UEDSkillBarWidget::ResolveItemData(const FPrimaryAssetId& ItemId) const
{
	return ResolveSkillBarItemData(ItemId);
}

FText UEDSkillBarWidget::ResolveItemDisplayName(const FPrimaryAssetId& ItemId) const
{
	if (const UEDInventoryItemDataAsset* ItemData = ResolveItemData(ItemId))
	{
		if (!ItemData->DisplayName.IsEmpty())
		{
			return ItemData->DisplayName;
		}
	}

	return ItemId.IsValid() ? FText::FromName(ItemId.PrimaryAssetName) : FText::FromString(TEXT("\uC2A4\uD0AC"));
}

bool UEDSkillBarWidget::ResolveCooldownFromTag(const FGameplayTag& CooldownTag, float& OutRemainingTime, float& OutMaxCooldownTime) const
{
	OutRemainingTime = 0.0f;
	OutMaxCooldownTime = 0.0f;

	if (!CachedAbilitySystemComponent || !CooldownTag.IsValid())
	{
		return false;
	}

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CooldownTag));
	const TArray<float> RemainingTimes = CachedAbilitySystemComponent->GetActiveEffectsTimeRemaining(Query);
	const TArray<float> MaxTimes = CachedAbilitySystemComponent->GetActiveEffectsDuration(Query);

	if (RemainingTimes.IsEmpty() || MaxTimes.IsEmpty())
	{
		return false;
	}

	OutRemainingTime = RemainingTimes[0];
	OutMaxCooldownTime = MaxTimes[0];
	return OutRemainingTime > KINDA_SMALL_NUMBER && OutMaxCooldownTime > KINDA_SMALL_NUMBER;
}

void UEDSkillBarWidget::RequestDeferredRefresh()
{
	bDeferredRefreshRequested = true;
}

void UEDSkillBarWidget::HandleSkillSlotDropped(EEDSkillSlotType TargetSkillSlotType, int32 SourceSlotIndex)
{
	if (!CachedInventoryComponent || SourceSlotIndex == INDEX_NONE)
	{
		return;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSucceeded = CachedInventoryComponent->PredicateEquipSkillFromSlot(SourceSlotIndex, TargetSkillSlotType, Failure);
	if (!bSucceeded)
	{
		ShowInventoryFailure(Failure);
		return;
	}

	FEDInventorySlotData SlotData;
	if (CachedInventoryComponent->InventorySlots.IsValidIndex(SourceSlotIndex))
	{
		SlotData = CachedInventoryComponent->InventorySlots[SourceSlotIndex];
	}

	ShowInventorySuccess(ResolveItemDisplayName(SlotData.Item.ItemId), FText::FromString(TEXT("\uC7A5\uCC29")));
}

void UEDSkillBarWidget::HandleFirstSkillSlotDropped(int32 SourceSlotIndex)
{
	HandleSkillSlotDropped(EEDSkillSlotType::FirstSkill, SourceSlotIndex);
}

void UEDSkillBarWidget::HandleSecondSkillSlotDropped(int32 SourceSlotIndex)
{
	HandleSkillSlotDropped(EEDSkillSlotType::SecondSkill, SourceSlotIndex);
}

void UEDSkillBarWidget::HandleSkillSlotDoubleClicked(EEDSkillSlotType SkillSlotType)
{
	if (!CachedInventoryComponent)
	{
		ShowInventoryFailure(EEDInventoryActionFailure::InvalidInventory);
		return;
	}

	if (!HasEmptyInventorySlot())
	{
		ShowInventoryFailure(EEDInventoryActionFailure::NoSpace);
		return;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSucceeded = (SkillSlotType == EEDSkillSlotType::FirstSkill)
		? CachedInventoryComponent->PredicateUnequipFirstSkill(Failure)
		: CachedInventoryComponent->PredicateUnequipSecondSkill(Failure);
	if (!bSucceeded)
	{
		ShowInventoryFailure(Failure);
		return;
	}

	ShowInventorySuccess(ResolveSkillItemDisplayName(SkillSlotType), FText::FromString(TEXT("\uD574\uC81C")));
}

void UEDSkillBarWidget::HandleFirstSkillSlotDoubleClicked()
{
	HandleSkillSlotDoubleClicked(EEDSkillSlotType::FirstSkill);
}

void UEDSkillBarWidget::HandleSecondSkillSlotDoubleClicked()
{
	HandleSkillSlotDoubleClicked(EEDSkillSlotType::SecondSkill);
}

void UEDSkillBarWidget::ShowInventoryFailure(EEDInventoryActionFailure Failure) const
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		return;
	}

	UIManageSubsystem->ShowToastMessage(
		EDUserFacingMessage::Inventory::GetFailureText(Failure),
		EEDUIMessageType::Error,
		3.0f);
}

void UEDSkillBarWidget::ShowInventorySuccess(const FText& TargetName, const FText& ActionName) const
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>();
	if (!UIManageSubsystem)
	{
		return;
	}

	UIManageSubsystem->ShowToastMessage(
		EDUserFacingMessage::Inventory::GetSuccessText(TargetName, ActionName),
		EEDUIMessageType::Success,
		3.0f);
}

bool UEDSkillBarWidget::HasEmptyInventorySlot() const
{
	if (!CachedInventoryComponent)
	{
		return false;
	}

	for (const FEDInventorySlotData& SlotData : CachedInventoryComponent->InventorySlots)
	{
		if (SlotData.IsEmpty())
		{
			return true;
		}
	}

	return false;
}

FText UEDSkillBarWidget::ResolveSkillItemDisplayName(EEDSkillSlotType SkillSlotType) const
{
	if (!CachedInventoryComponent)
	{
		return FText::FromString(TEXT("\uC2A4\uD0AC"));
	}

	FPrimaryAssetId ItemId;
	if (SkillSlotType == EEDSkillSlotType::FirstSkill)
	{
		ItemId = CachedInventoryComponent->FirstSkillSlot.EquippedItem.ItemId;
	}
	else
	{
		ItemId = CachedInventoryComponent->SecondSkillSlot.EquippedItem.ItemId;
	}

	return ResolveItemDisplayName(ItemId);
}

void UEDSkillBarWidget::HandleWeaponSlotChanged(const FGameplayTagContainer& MainItemTags, const FGameplayTagContainer& SpecialItemTags, const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags)
{
	RequestDeferredRefresh();
}

void UEDSkillBarWidget::HandleFirstSkillSlotChanged(const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags)
{
	RequestDeferredRefresh();
}

void UEDSkillBarWidget::HandleSecondSkillSlotChanged(const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags)
{
	RequestDeferredRefresh();
}

void UEDSkillBarWidget::HandleQSkillCoolTime(float SkillCoolTime, float MaxSkillCoolTime)
{
	if (!QSkillSlot)
	{
		return;
	}

	if (SkillCoolTime > KINDA_SMALL_NUMBER && MaxSkillCoolTime > KINDA_SMALL_NUMBER)
	{
		QSkillSlot->StartCooldown(SkillCoolTime, MaxSkillCoolTime);
	}
	else
	{
		QSkillSlot->ClearCooldown();
	}
}

void UEDSkillBarWidget::HandleESkillCoolTime(float SkillCoolTime, float MaxSkillCoolTime)
{
	if (!ESkillSlot)
	{
		return;
	}

	if (SkillCoolTime > KINDA_SMALL_NUMBER && MaxSkillCoolTime > KINDA_SMALL_NUMBER)
	{
		ESkillSlot->StartCooldown(SkillCoolTime, MaxSkillCoolTime);
	}
	else
	{
		ESkillSlot->ClearCooldown();
	}
}

void UEDSkillBarWidget::HandleSpaceSkillCoolTime(float SkillCoolTime, float MaxSkillCoolTime)
{
	if (!SpaceSkillSlot)
	{
		return;
	}

	if (SkillCoolTime > KINDA_SMALL_NUMBER && MaxSkillCoolTime > KINDA_SMALL_NUMBER)
	{
		SpaceSkillSlot->StartCooldown(SkillCoolTime, MaxSkillCoolTime);
	}
	else
	{
		SpaceSkillSlot->ClearCooldown();
	}
}
