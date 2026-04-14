// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDInventoryQuantityPopupWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UEDInventoryQuantityPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MinusButton)
	{
		MinusButton->OnClicked.AddUniqueDynamic(this, &UEDInventoryQuantityPopupWidget::HandleMinusClicked);
	}

	if (PlusButton)
	{
		PlusButton->OnClicked.AddUniqueDynamic(this, &UEDInventoryQuantityPopupWidget::HandlePlusClicked);
	}

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddUniqueDynamic(this, &UEDInventoryQuantityPopupWidget::HandleConfirmClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddUniqueDynamic(this, &UEDInventoryQuantityPopupWidget::HandleCancelClicked);
	}

	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("버릴 수량 선택")));
	}

	ResetPopupState();
}

void UEDInventoryQuantityPopupWidget::SetupQuantityRange(int32 InMinQuantity, int32 InMaxQuantity,
                                                         int32 InInitialQuantity)
{
	MinQuantity = FMath::Max(1, InMinQuantity);
	MaxQuantity = FMath::Max(MinQuantity, InMaxQuantity);
	CurrentQuantity = FMath::Clamp(InInitialQuantity, MinQuantity, MaxQuantity);

	SetVisibility(ESlateVisibility::Visible);
	RefreshQuantityDisplay();
}

int32 UEDInventoryQuantityPopupWidget::GetCurrentQuantity() const
{
	return CurrentQuantity;
}

void UEDInventoryQuantityPopupWidget::ResetPopupState()
{
	MinQuantity = 1;
	MaxQuantity = 1;
	CurrentQuantity = 1;

	SetVisibility(ESlateVisibility::Collapsed);
	RefreshQuantityDisplay();
}

void UEDInventoryQuantityPopupWidget::RefreshQuantityDisplay()
{
	if (QuantityText)
	{
		QuantityText->SetText(FText::AsNumber(CurrentQuantity));
	}

	if (MinusButton)
	{
		MinusButton->SetIsEnabled(CurrentQuantity > MinQuantity);
	}

	if (PlusButton)
	{
		PlusButton->SetIsEnabled(CurrentQuantity < MaxQuantity);
	}

	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(CurrentQuantity >= MinQuantity && CurrentQuantity <= MaxQuantity);
	}
}

void UEDInventoryQuantityPopupWidget::HandleMinusClicked()
{
	CurrentQuantity = FMath::Max(MinQuantity, CurrentQuantity - 1);
	RefreshQuantityDisplay();
}

void UEDInventoryQuantityPopupWidget::HandlePlusClicked()
{
	CurrentQuantity = FMath::Min(MaxQuantity, CurrentQuantity + 1);
	RefreshQuantityDisplay();
}

void UEDInventoryQuantityPopupWidget::HandleConfirmClicked()
{
	OnQuantityConfirmed.Broadcast(CurrentQuantity);
}

void UEDInventoryQuantityPopupWidget::HandleCancelClicked()
{
	OnQuantityCanceled.Broadcast();
}
