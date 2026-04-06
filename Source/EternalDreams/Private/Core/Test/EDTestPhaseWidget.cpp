// Copyright Eternal Dreams Team. All Rights Reserved.

#include "Core/Test/EDTestPhaseWidget.h"
#include "Core/EDGameState.h"
#include "Core/EDPlayerController_Temp.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UEDTestPhaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SkipPhaseButton->OnClicked.AddDynamic(this, &UEDTestPhaseWidget::OnSkipPhaseClicked);

	UpdateDisplay();

}

void UEDTestPhaseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateDisplay();
}

void UEDTestPhaseWidget::OnSkipPhaseClicked()
{
	AEDPlayerController_Temp* PC = Cast<AEDPlayerController_Temp>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}

	PC->Server_RequestSkipPhase();
}

void UEDTestPhaseWidget::UpdateDisplay()
{
	// GameState는 클라이언트에도 리플리케이트되므로 직접 접근 가능
	const AEDGameState* GS = GetWorld()->GetGameState<AEDGameState>();
	if (!GS)
	{
		PhaseNameText->SetText(FText::FromString(TEXT("대기 중")));
		RemainingTimeText->SetText(FText::FromString(TEXT("--:--")));
		PhaseIndexText->SetText(FText::FromString(TEXT("")));
		return;
	}

	// 페이즈 이름
	const FGameplayTag CurrentPhase = GS->GetCurrentPhase();
	if (CurrentPhase.IsValid())
	{
		PhaseNameText->SetText(FText::FromString(CurrentPhase.ToString()));
	}
	else
	{
		PhaseNameText->SetText(FText::FromString(TEXT("페이즈 없음")));
	}

	// 남은 시간 (MM:SS)
	const float Remaining = GS->GetPhaseRemainingTime();
	const int32 Minutes = FMath::FloorToInt(Remaining / 60.f);
	const int32 Seconds = FMath::FloorToInt(FMath::Fmod(Remaining, 60.f));
	RemainingTimeText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));

	// 페이즈 태그 표시
	PhaseIndexText->SetText(FText::FromString(
		FString::Printf(TEXT("Phase: %s"), *CurrentPhase.ToString())));
}
