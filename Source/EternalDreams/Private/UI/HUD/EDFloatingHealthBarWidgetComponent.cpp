#include "UI/HUD/EDFloatingHealthBarWidgetComponent.h"

#include "Characters/Monster/EDMonsterBase.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "UI/HUD/EDFloatingHealthBarWidget.h"

void UEDFloatingHealthBarWidgetComponent::InitWidget()
{
	Super::InitWidget();

	InitializeFromOwner();
}

void UEDFloatingHealthBarWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeFromOwner();
}

void UEDFloatingHealthBarWidgetComponent::InitializeFromOwner()
{
	UEDFloatingHealthBarWidget* FloatingHealthBarWidget = Cast<UEDFloatingHealthBarWidget>(GetUserWidgetObject());
	if (IsValid(FloatingHealthBarWidget) == false)
	{
		return;
	}

	AEDPlayerCharacter* PlayerCharacter = Cast<AEDPlayerCharacter>(GetOwner());
	if (IsValid(PlayerCharacter))
	{
		const bool bShouldShowFloatingHealthBar = PlayerCharacter->IsLocallyControlled() == false;
		SetVisibility(bShouldShowFloatingHealthBar);
		if (bShouldShowFloatingHealthBar == false)
		{
			return;
		}

		if (bSourceBound == false)
		{
			PlayerCharacter->GetOnFloatingHealthBarSourceChanged().AddUObject(FloatingHealthBarWidget, &UEDFloatingHealthBarWidget::InitializeHealthSource);
			bSourceBound = true;
		}

		FloatingHealthBarWidget->InitializeHealthSource(PlayerCharacter->GetAbilitySystemComponent(), PlayerCharacter->GetBaseAttributeSet());
		return;
	}

	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(GetOwner());
	if (IsValid(Monster) == false)
	{
		return;
	}

	SetVisibility(true);

	if (bSourceBound == false)
	{
		Monster->GetOnFloatingHealthBarSourceChanged().AddUObject(FloatingHealthBarWidget, &UEDFloatingHealthBarWidget::InitializeHealthSource);
		bSourceBound = true;
	}

	FloatingHealthBarWidget->InitializeHealthSource(Monster->GetAbilitySystemComponent(), Monster->GetBaseAttributeSet());
}
