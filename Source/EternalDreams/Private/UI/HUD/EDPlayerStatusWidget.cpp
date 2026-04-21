// Copyright Epic Games, Inc. All Rights Reserved.
#include "UI/HUD/EDPlayerStatusWidget.h"

#include "AbilitySystemComponent.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Core/EDGameInstance.h"
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

FText GetUnavailableText()
{
	return FText::FromString(TEXT("-"));
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

void UEDPlayerStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshPlayerReferencesIfNeeded();
	RefreshDeferredDisplayIfNeeded();
}

void UEDPlayerStatusWidget::InitializePlayerReferences()
{
	CachedPlayerCharacter = Cast<AEDPlayerCharacter>(GetOwningPlayerPawn());
	CachedPlayerState = CachedPlayerCharacter ? CachedPlayerCharacter->GetPlayerState<AEDPlayerState>() : nullptr;
	CachedAbilitySystemComponent = CachedPlayerCharacter ? CachedPlayerCharacter->GetAbilitySystemComponent() : nullptr;
	CachedBaseAttributeSet = CachedPlayerCharacter ? CachedPlayerCharacter->GetBaseAttributeSet() : nullptr;
	CachedPlayerAttributeSet = CachedPlayerCharacter ? CachedPlayerCharacter->GetPlayerAttributeSet() : nullptr;
}

void UEDPlayerStatusWidget::RefreshPlayerReferencesIfNeeded()
{
	AEDPlayerCharacter* CurrentPlayerCharacter = Cast<AEDPlayerCharacter>(GetOwningPlayerPawn());
	AEDPlayerState* CurrentPlayerState = CurrentPlayerCharacter ? CurrentPlayerCharacter->GetPlayerState<AEDPlayerState>() : nullptr;
	UAbilitySystemComponent* CurrentAbilitySystemComponent = CurrentPlayerCharacter ? CurrentPlayerCharacter->GetAbilitySystemComponent() : nullptr;
	UEDBaseAttributeSet* CurrentBaseAttributeSet = CurrentPlayerCharacter ? CurrentPlayerCharacter->GetBaseAttributeSet() : nullptr;
	UEDPlayerAttributeSet* CurrentPlayerAttributeSet = CurrentPlayerCharacter ? CurrentPlayerCharacter->GetPlayerAttributeSet() : nullptr;

	if (CachedPlayerCharacter == CurrentPlayerCharacter &&
		CachedPlayerState == CurrentPlayerState &&
		CachedAbilitySystemComponent == CurrentAbilitySystemComponent &&
		CachedBaseAttributeSet == CurrentBaseAttributeSet &&
		CachedPlayerAttributeSet == CurrentPlayerAttributeSet)
	{
		return;
	}

	UnbindAttributeDelegates();
	InitializePlayerReferences();
	BindAttributeDelegates();
	RefreshAllDisplay();
}

void UEDPlayerStatusWidget::RefreshDeferredDisplayIfNeeded() const
{
	if (!PlayerNameText || !CachedPlayerState)
	{
		return;
	}

	const FString CurrentPlayerName = CachedPlayerState->GetPlayerName();
	if (CurrentPlayerName.IsEmpty())
	{
		return;
	}

	if (PlayerNameText->GetText().ToString() != CurrentPlayerName)
	{
		RefreshPlayerName();
	}
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

	if (const UEDGameInstance* GI = GetGameInstance<UEDGameInstance>())
	{
		const FString Nickname = GI->LocalPlayerNickname.TrimStartAndEnd();
		if (!Nickname.IsEmpty())
		{
			PlayerNameText->SetText(FText::FromString(Nickname));
			return;
		}
	}

	if (CachedPlayerState)
	{
		const FString DisplayNickname = CachedPlayerState->GetDisplayNickname().TrimStartAndEnd();
		if (!DisplayNickname.IsEmpty())
		{
			PlayerNameText->SetText(FText::FromString(DisplayNickname));
			return;
		}

		const FString PlayerName = CachedPlayerState->GetPlayerName().TrimStartAndEnd();
		if (!PlayerName.IsEmpty())
		{
			PlayerNameText->SetText(FText::FromString(PlayerName));
			return;
		}
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
	if (DefensiveValueText)
	{
		if (!CachedBaseAttributeSet)
		{
			DefensiveValueText->SetText(GetUnavailableText());
			return;
		}

		const float CurrentDefensive = CachedBaseAttributeSet->GetDefensive();
		DefensiveValueText->SetText(FText::AsNumber(FMath::RoundToInt(CurrentDefensive)));
	}
}

void UEDPlayerStatusWidget::RefreshMoveSpeedDisplay() const
{
	if (!MoveSpeedValueText)
	{
		return;
	}

	if (!CachedBaseAttributeSet)
	{
		MoveSpeedValueText->SetText(GetUnavailableText());
		return;
	}

	const float WalkSpeed = CachedBaseAttributeSet->GetWalkSpeed();
	MoveSpeedValueText->SetText(FText::AsNumber(FMath::RoundToInt(WalkSpeed)));
}

void UEDPlayerStatusWidget::RefreshStatDisplay() const
{
	if (StrengthValueText)
	{
		StrengthValueText->SetText(CachedPlayerAttributeSet
			                           ? FText::AsNumber(FMath::RoundToInt(CachedPlayerAttributeSet->GetStrength()))
			                           : GetUnavailableText());
	}

	if (DexterityValueText)
	{
		DexterityValueText->SetText(CachedPlayerAttributeSet
			                            ? FText::AsNumber(FMath::RoundToInt(CachedPlayerAttributeSet->GetDexterity()))
			                            : GetUnavailableText());
	}

	if (IntelligenceValueText)
	{
		IntelligenceValueText->SetText(CachedPlayerAttributeSet
			                               ? FText::AsNumber(FMath::RoundToInt(CachedPlayerAttributeSet->GetIntelligence()))
			                               : GetUnavailableText());
	}
}

void UEDPlayerStatusWidget::RefreshAttackSpeedDisplay() const
{
	if (AttackSpeedValueText)
	{
		AttackSpeedValueText->SetText(GetUnavailableText());
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
