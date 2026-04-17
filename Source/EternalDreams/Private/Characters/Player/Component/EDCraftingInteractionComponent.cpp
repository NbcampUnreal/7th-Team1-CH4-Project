// Copyright Epic Games, Inc. All Rights Reserved.
#include "Characters/Player/Component/EDCraftingInteractionComponent.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/BP/EDInventoryBlueprintLibrary.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "UI/Message/EDUserFacingMessage.h"
#include "UI/Subsystem/EDUIManageSubsystem.h"
#include "UI/Types/EDUITypes.h"

UEDCraftingInteractionComponent::UEDCraftingInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEDCraftingInteractionComponent::HandleCraftInput()
{
	UEDInventoryComponent* InventoryComponent = GetOwningInventoryComponent();
	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDCraftingInteractionComponent: InventoryComponent를 찾지 못해 제작 입력을 처리할 수 없습니다."));
		return;
	}

	InventoryComponent->RefreshCraftableRecipesCache();

	FEDCraftableRecipeEntry CraftTargetRecipe;
	if (!TryGetFirstCraftableRecipeEntry(InventoryComponent, CraftTargetRecipe))
	{
		if (APlayerController* PlayerController = GetOwningPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
				{
					UIManageSubsystem->ShowToastMessage(
						EDUserFacingMessage::Craft::GetFailureText(EEDInventoryActionFailure::InvalidRecipe),
						EEDUIMessageType::Error,
						3.0f);
				}
			}
		}
		return;
	}

	EEDInventoryActionFailure Failure = EEDInventoryActionFailure::None;
	const bool bSucceeded = InventoryComponent->PredicateCraftItem(CraftTargetRecipe.RecipeId, Failure);
	if (!bSucceeded)
	{
		if (APlayerController* PlayerController = GetOwningPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
				{
					UIManageSubsystem->ShowToastMessage(
						EDUserFacingMessage::Craft::GetFailureText(Failure),
						EEDUIMessageType::Error,
						3.0f);
				}
			}
		}
	}
	else
	{
		if (APlayerController* PlayerController = GetOwningPlayerController())
		{
			if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
			{
				if (UEDUIManageSubsystem* UIManageSubsystem = LocalPlayer->GetSubsystem<UEDUIManageSubsystem>())
				{
					UIManageSubsystem->ShowToastMessage(
						EDUserFacingMessage::Craft::GetSuccessText(CraftTargetRecipe.ResultItemName),
						EEDUIMessageType::Success,
						3.0f);
				}
			}
		}
	}
}

APlayerController* UEDCraftingInteractionComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

UEDInventoryComponent* UEDCraftingInteractionComponent::GetOwningInventoryComponent() const
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	APawn* ControlledPawn = PlayerController->GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	return UEDInventoryBlueprintLibrary::GetInventoryComponentFromActor(ControlledPawn);
}

bool UEDCraftingInteractionComponent::TryGetFirstCraftableRecipeEntry(
	UEDInventoryComponent* InventoryComponent,
	FEDCraftableRecipeEntry& OutRecipeEntry) const
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

	OutRecipeEntry = CraftableRecipeEntries[0];
	return true;
}


