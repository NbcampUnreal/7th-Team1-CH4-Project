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
	// To-Do: 하드 코딩 제거 후, PlayerAttributeSet에서 데이터 가져오도록 수정
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(TEXT("Player")));
	}

	if (LevelText)
	{
		LevelText->SetText(FText::FromString(TEXT("Lv.14")));
	}

	if (HPBar)
	{
		HPBar->SetPercent(0.85f);
	}

	if (HPValueText)
	{
		HPValueText->SetText(FText::FromString(TEXT("850 / 1000")));
	}

	if (ManaBar)
	{
		ManaBar->SetPercent(0.8f);
	}

	if (ManaValueText)
	{
		ManaValueText->SetText(FText::FromString(TEXT("120 / 150")));
	}

	if (StatusText)
	{
		StatusText->SetText(FText::FromString(TEXT("상태: Normal")));
	}
	
	UE_LOG(LogTemp, Log, TEXT("EDPlayerStatusWidget: 테스트 데이터 적용"));
}
