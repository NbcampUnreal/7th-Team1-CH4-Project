// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/EDTimerWidget.h"
#include "Components/TextBlock.h"

void UEDTimerWidget::UpdateTimeText(float InRemainingTime)
{
	if (!TimeText)
	{
		return;
	}

	// 남은시간 클램프
	float DisplayTime = FMath::Max(0.0f, InRemainingTime);

	// 소수점 0자리까지 표시
	FString TimeString = FString::Printf(TEXT("%.f"), DisplayTime);

	TimeText->SetText(FText::FromString(TimeString));
}
