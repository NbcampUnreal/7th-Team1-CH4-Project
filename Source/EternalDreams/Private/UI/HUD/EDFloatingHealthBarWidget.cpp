#include "UI/HUD/EDFloatingHealthBarWidget.h"

#include "AbilitySystemComponent.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

namespace
{
	FText FormatHealthText(float InCurrentHealth, float InMaxHealth)
	{
		return FText::Format(
			NSLOCTEXT("FloatingHealthBar", "HealthFormat", "{0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(InCurrentHealth)),
			FText::AsNumber(FMath::RoundToInt(InMaxHealth)));
	}
}

void UEDFloatingHealthBarWidget::NativeDestruct()
{
	UnbindHealthDelegates();

	Super::NativeDestruct();
}

void UEDFloatingHealthBarWidget::InitializeHealthSource(
	UAbilitySystemComponent* InAbilitySystemComponent,
	UEDBaseAttributeSet* InBaseAttributeSet)
{
	if (CachedAbilitySystemComponent == InAbilitySystemComponent && CachedBaseAttributeSet == InBaseAttributeSet)
	{
		RefreshHealthDisplay();
		return;
	}

	UnbindHealthDelegates();

	CachedAbilitySystemComponent = InAbilitySystemComponent;
	CachedBaseAttributeSet = InBaseAttributeSet;

	BindHealthDelegates();
	RefreshHealthDisplay();
}

void UEDFloatingHealthBarWidget::BindHealthDelegates()
{
	if (!CachedAbilitySystemComponent || !CachedBaseAttributeSet)
	{
		return;
	}

	HealthChangedHandle = CachedAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UEDFloatingHealthBarWidget::HandleHealthChanged);

	MaxHealthChangedHandle = CachedAbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UEDFloatingHealthBarWidget::HandleMaxHealthChanged);
}

void UEDFloatingHealthBarWidget::UnbindHealthDelegates()
{
	if (!CachedAbilitySystemComponent)
	{
		return;
	}

	if (HealthChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		HealthChangedHandle.Reset();
	}

	if (MaxHealthChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		MaxHealthChangedHandle.Reset();
	}
}

void UEDFloatingHealthBarWidget::RefreshHealthDisplay() const
{
	if (!CachedBaseAttributeSet)
	{
		if (HPBar)
		{
			HPBar->SetPercent(0.0f);
		}

		if (HPValueText)
		{
			HPValueText->SetText(FText::FromString(TEXT("-")));
		}

		return;
	}

	const float CurrentHealth = CachedBaseAttributeSet->GetHealth();
	const float MaxHealth = CachedBaseAttributeSet->GetMaxHealth();
	const float HealthPercent = MaxHealth > KINDA_SMALL_NUMBER
		? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 0.0f;

	if (HPBar)
	{
		HPBar->SetPercent(HealthPercent);
	}

	if (HPValueText)
	{
		HPValueText->SetText(FormatHealthText(CurrentHealth, MaxHealth));
	}
}

void UEDFloatingHealthBarWidget::HandleHealthChanged(const FOnAttributeChangeData& Data) const
{
	RefreshHealthDisplay();
}

void UEDFloatingHealthBarWidget::HandleMaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	RefreshHealthDisplay();
}
