// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDPlayerStatusWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEDPlayerStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyTestData();
}

void UEDPlayerStatusWidget::ApplyTestData() const
{
	if (HPBar)
	{
		HPBar->SetPercent(0.75f);
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("HP 75 / 100")));
	}

	UE_LOG(LogTemp, Log, TEXT("EDPlayerStatusWidget: 테스트 데이터 적용"));
}
