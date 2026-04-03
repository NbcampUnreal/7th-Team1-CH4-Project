// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/Panel/EDPauseMenuWidget.h"

#include "Components/TextBlock.h"

void UEDPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyPreviewText();

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: 일시정지 메뉴 패널이 생성되었습니다."));
}

void UEDPauseMenuWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: 일시정지 메뉴 패널이 열렸습니다."));
}

void UEDPauseMenuWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UE_LOG(LogTemp, Log, TEXT("EDPauseMenuWidget: 일시정지 메뉴 패널이 닫혔습니다."));
}

void UEDPauseMenuWidget::ApplyPreviewText() const
{
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("일시정지 메뉴")));
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(FText::FromString(TEXT("ESC 입력 처리 확인용 기본 패널입니다.")));
	}
}
