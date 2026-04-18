// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDToastMessageWidget.h"

#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

void UEDToastMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::Collapsed);
	SetRenderOpacity(0.0f);
	ApplyToastLayout();
}

void UEDToastMessageWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsShowing)
	{
		return;
	}

	ElapsedTime += InDeltaTime;

	float TargetOpacity = 1.0f;

	// 처음 짧은 구간은 부드럽게 나타남
	if (ElapsedTime < FadeInDuration)
	{
		TargetOpacity = FMath::Clamp(ElapsedTime / FadeInDuration, 0.0f, 1.0f);
	}
	// 마지막 구간은 부드럽게 사라짐
	else if (ElapsedTime > DisplayDuration - FadeOutDuration)
	{
		const float FadeOutElapsed = ElapsedTime - (DisplayDuration - FadeOutDuration);
		TargetOpacity = 1.0f - FMath::Clamp(FadeOutElapsed / FadeOutDuration, 0.0f, 1.0f);
	}

	SetRenderOpacity(TargetOpacity);

	if (ElapsedTime >= DisplayDuration)
	{
		bIsShowing = false;
		SetRenderOpacity(0.0f);
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEDToastMessageWidget::ShowToastMessage(const FText& InMessage, EEDUIMessageType InMessageType, float InDuration)
{
	DisplayDuration = FMath::Max(0.8f, InDuration);
	ElapsedTime = 0.0f;
	bIsShowing = true;

	if (MessageText)
	{
		MessageText->SetText(InMessage);
		MessageText->SetColorAndOpacity(GetTextColor(InMessageType));
	}

	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(GetBackgroundColor(InMessageType));
	}

	ApplyToastLayout();
	InvalidateLayoutAndVolatility();
	ForceLayoutPrepass();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetRenderOpacity(0.0f);
}

void UEDToastMessageWidget::ApplyToastLayout()
{
	if (RootSizeBox)
	{
		RootSizeBox->SetWidthOverride(ToastWidth);
		RootSizeBox->SetMinDesiredWidth(ToastWidth);
		RootSizeBox->SetMaxDesiredWidth(ToastWidth);
		RootSizeBox->ClearHeightOverride();
		RootSizeBox->ClearMinDesiredHeight();
		RootSizeBox->ClearMaxDesiredHeight();
	}

	if (BackgroundBorder)
	{
		BackgroundBorder->SetPadding(FMargin(18.0f, 12.0f));
		BackgroundBorder->SetClipping(EWidgetClipping::Inherit);
	}

	if (MessageText)
	{
		MessageText->SetAutoWrapText(true);
		MessageText->SetWrapTextAt(TextWrapWidth);
		MessageText->SetJustification(ETextJustify::Center);
		MessageText->SetClipping(EWidgetClipping::Inherit);
		MessageText->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);
	}
}

FLinearColor UEDToastMessageWidget::GetBackgroundColor(EEDUIMessageType InMessageType) const
{
	switch (InMessageType)
	{
	case EEDUIMessageType::Success:
		return FLinearColor(0.10f, 0.32f, 0.16f, 0.72f);
	case EEDUIMessageType::Warning:
		return FLinearColor(0.42f, 0.28f, 0.08f, 0.72f);
	case EEDUIMessageType::Error:
		return FLinearColor(0.36f, 0.11f, 0.11f, 0.76f);
	case EEDUIMessageType::Info:
	default:
		return FLinearColor(0.10f, 0.10f, 0.10f, 0.68f);
	}
}

FLinearColor UEDToastMessageWidget::GetTextColor(EEDUIMessageType InMessageType) const
{
	switch (InMessageType)
	{
	case EEDUIMessageType::Success:
		return FLinearColor(0.90f, 1.00f, 0.92f, 1.0f);
	case EEDUIMessageType::Warning:
		return FLinearColor(1.00f, 0.95f, 0.82f, 1.0f);
	case EEDUIMessageType::Error:
		return FLinearColor(1.00f, 0.90f, 0.90f, 1.0f);
	case EEDUIMessageType::Info:
	default:
		return FLinearColor(0.96f, 0.96f, 0.96f, 1.0f);
	}
}
