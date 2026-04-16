// Copyright Epic Games, Inc. All Rights Reserved.
#include "Characters/Player/Component/EDCraftingInteractionComponent.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "UI/HUD/EDItemCraftingWidget.h"

UEDCraftingInteractionComponent::UEDCraftingInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEDCraftingInteractionComponent::HandleCraftInput()
{
	UEDItemCraftingWidget* CraftingWidget = FindCraftingWidget();
	if (!CraftingWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("EDCraftingInteractionComponent: 제작 위젯을 찾지 못해 제작 입력을 처리할 수 없습니다."));
		return;
	}

	CraftingWidget->RequestCraftSelectedRecipe();
}

APlayerController* UEDCraftingInteractionComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

UEDItemCraftingWidget* UEDCraftingInteractionComponent::FindCraftingWidget() const
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
		PlayerController,
		FoundWidgets,
		UEDItemCraftingWidget::StaticClass(),
		false);

	for (UUserWidget* Widget : FoundWidgets)
	{
		UEDItemCraftingWidget* CraftingWidget = Cast<UEDItemCraftingWidget>(Widget);
		if (CraftingWidget && CraftingWidget->IsVisible())
		{
			return CraftingWidget;
		}
	}

	return FoundWidgets.Num() > 0 ? Cast<UEDItemCraftingWidget>(FoundWidgets[0]) : nullptr;
}
