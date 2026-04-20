// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDPlayerStatusWidget.h"

#include "AbilitySystemComponent.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/EDPlayerState.h"

namespace
{
FText FormatCurrentMaxText(float CurrentValue, float MaxValue)
{
	return FText::Format(
		NSLOCTEXT("PlayerStatus", "CurrentMaxFormat", "{0} / {1}"),
		FText::AsNumber(FMath::RoundToInt(CurrentValue)),
		FText::AsNumber(FMath::RoundToInt(MaxValue)));
}
}

void UEDPlayerStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializePlayerReferences();
	BindAttributeDelegates();
	RefreshAllDisplay();
}

void UEDPlayerStatusWidget::NativeDestruct()
{
	UnbindAttributeDelegates();

	Super::NativeDestruct();
}

void UEDPlayerStatusWidget::InitializePlayerReferences()
{
	CachedPlayerCharacter = Cast<AEDPlayerCharacter>(GetOwningPlayerPawn());
	CachedPlayerState = CachedPlayerCharacter ? CachedPlayerCharacter->GetPlayerState<AEDPlayerState>() : nullptr;
	CachedAbilitySystemComponent = CachedPlayerCharacter ? CachedPlayerCharacter->GetAbilitySystemComponent() : nullptr;
	CachedBaseAttributeSet = CachedPlayerCharacter ? CachedPlayerCharacter->GetBaseAttributeSet() : nullptr;
	CachedPlayerAttributeSet = CachedPlayerCharacter ? CachedPlayerCharacter->GetPlayerAttributeSet() : nullptr;
}

void UEDPlayerStatusWidget::BindAttributeDelegates()
{
	UnbindAttributeDelegates();

	if (!CachedAbilitySystemComponent)
	{
		return;
	}

	if (CachedBaseAttributeSet)
	{
		HealthChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetHealthAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleHealthChanged);

		MaxHealthChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleMaxHealthChanged);

		DefensiveChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetDefensiveAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleDefensiveChanged);

		MaxDefensiveChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetMaxDefensiveAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleMaxDefensiveChanged);

		WalkSpeedChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetWalkSpeedAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleWalkSpeedChanged);
	}

	if (CachedPlayerAttributeSet)
	{
		StrengthChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDPlayerAttributeSet::GetStrengthAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleStrengthChanged);

		DexterityChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDPlayerAttributeSet::GetDexterityAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleDexterityChanged);

		IntelligenceChangedHandle = CachedAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UEDPlayerAttributeSet::GetIntelligenceAttribute())
			.AddUObject(this, &UEDPlayerStatusWidget::HandleIntelligenceChanged);
	}
}

void UEDPlayerStatusWidget::UnbindAttributeDelegates()
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

	if (DefensiveChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetDefensiveAttribute()).Remove(DefensiveChangedHandle);
		DefensiveChangedHandle.Reset();
	}

	if (MaxDefensiveChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetMaxDefensiveAttribute()).Remove(MaxDefensiveChangedHandle);
		MaxDefensiveChangedHandle.Reset();
	}

	if (WalkSpeedChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetWalkSpeedAttribute()).Remove(WalkSpeedChangedHandle);
		WalkSpeedChangedHandle.Reset();
	}

	if (StrengthChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDPlayerAttributeSet::GetStrengthAttribute()).Remove(StrengthChangedHandle);
		StrengthChangedHandle.Reset();
	}

	if (DexterityChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDPlayerAttributeSet::GetDexterityAttribute()).Remove(DexterityChangedHandle);
		DexterityChangedHandle.Reset();
	}

	if (IntelligenceChangedHandle.IsValid())
	{
		CachedAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDPlayerAttributeSet::GetIntelligenceAttribute()).Remove(IntelligenceChangedHandle);
		IntelligenceChangedHandle.Reset();
	}
}

void UEDPlayerStatusWidget::RefreshAllDisplay() const
{
	RefreshPlayerName();
	RefreshPortrait();
	RefreshHealthDisplay();
	RefreshDefensiveDisplay();
	RefreshMoveSpeedDisplay();
	RefreshStatDisplay();
	RefreshAttackSpeedDisplay();
}

void UEDPlayerStatusWidget::RefreshPlayerName() const
{
	if (!PlayerNameText)
	{
		return;
	}

	if (CachedPlayerState)
	{
		PlayerNameText->SetText(FText::FromString(CachedPlayerState->GetPlayerName()));
		return;
	}

	if (CachedPlayerCharacter)
	{
		PlayerNameText->SetText(FText::FromString(CachedPlayerCharacter->GetName()));
		return;
	}

	PlayerNameText->SetText(FText::GetEmpty());
}

void UEDPlayerStatusWidget::RefreshPortrait() const
{
	// 수정 필요: 플레이어 이미지 슬롯
	if (PlayerImage)
	{
		PlayerImage->SetBrushFromTexture(nullptr);
	}
}

void UEDPlayerStatusWidget::RefreshHealthDisplay() const
{
	const float CurrentHealth = CachedBaseAttributeSet ? CachedBaseAttributeSet->GetHealth() : 0.0f;
	const float MaxHealth = CachedBaseAttributeSet ? CachedBaseAttributeSet->GetMaxHealth() : 0.0f;
	const float HealthPercent = MaxHealth > KINDA_SMALL_NUMBER ? CurrentHealth / MaxHealth : 0.0f;

	if (HPBar)
	{
		HPBar->SetPercent(HealthPercent);
	}

	if (HPValueText)
	{
		HPValueText->SetText(FormatCurrentMaxText(CurrentHealth, MaxHealth));
	}
}

void UEDPlayerStatusWidget::RefreshDefensiveDisplay() const
{
	const float CurrentDefensive = CachedBaseAttributeSet ? CachedBaseAttributeSet->GetDefensive() : 0.0f;
	const float MaxDefensive = CachedBaseAttributeSet ? CachedBaseAttributeSet->GetMaxDefensive() : 0.0f;

	if (DefensiveValueText)
	{
		DefensiveValueText->SetText(FormatCurrentMaxText(CurrentDefensive, MaxDefensive));
	}
}

void UEDPlayerStatusWidget::RefreshMoveSpeedDisplay() const
{
	if (!MoveSpeedValueText)
	{
		return;
	}

	const float WalkSpeed = CachedBaseAttributeSet ? CachedBaseAttributeSet->GetWalkSpeed() : 0.0f;
	MoveSpeedValueText->SetText(FText::AsNumber(FMath::RoundToInt(WalkSpeed)));
}

void UEDPlayerStatusWidget::RefreshStatDisplay() const
{
	const int32 Strength = CachedPlayerAttributeSet ? FMath::RoundToInt(CachedPlayerAttributeSet->GetStrength()) : 0;
	const int32 Dexterity = CachedPlayerAttributeSet ? FMath::RoundToInt(CachedPlayerAttributeSet->GetDexterity()) : 0;
	const int32 Intelligence = CachedPlayerAttributeSet ? FMath::RoundToInt(CachedPlayerAttributeSet->GetIntelligence()) : 0;

	if (StrengthValueText)
	{
		StrengthValueText->SetText(FText::AsNumber(Strength));
	}

	if (DexterityValueText)
	{
		DexterityValueText->SetText(FText::AsNumber(Dexterity));
	}

	if (IntelligenceValueText)
	{
		IntelligenceValueText->SetText(FText::AsNumber(Intelligence));
	}
}

void UEDPlayerStatusWidget::RefreshAttackSpeedDisplay() const
{
	if (AttackSpeedValueText)
	{
		AttackSpeedValueText->SetText(FText::GetEmpty());
	}
}

void UEDPlayerStatusWidget::HandleHealthChanged(const FOnAttributeChangeData& Data) const
{
	RefreshHealthDisplay();
}

void UEDPlayerStatusWidget::HandleMaxHealthChanged(const FOnAttributeChangeData& Data) const
{
	RefreshHealthDisplay();
}

void UEDPlayerStatusWidget::HandleDefensiveChanged(const FOnAttributeChangeData& Data) const
{
	RefreshDefensiveDisplay();
}

void UEDPlayerStatusWidget::HandleMaxDefensiveChanged(const FOnAttributeChangeData& Data) const
{
	RefreshDefensiveDisplay();
}

void UEDPlayerStatusWidget::HandleWalkSpeedChanged(const FOnAttributeChangeData& Data) const
{
	RefreshMoveSpeedDisplay();
}

void UEDPlayerStatusWidget::HandleStrengthChanged(const FOnAttributeChangeData& Data) const
{
	RefreshStatDisplay();
}

void UEDPlayerStatusWidget::HandleDexterityChanged(const FOnAttributeChangeData& Data) const
{
	RefreshStatDisplay();
}

void UEDPlayerStatusWidget::HandleIntelligenceChanged(const FOnAttributeChangeData& Data) const
{
	RefreshStatDisplay();
}
