#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayEffectTypes.h"
#include "EDFloatingHealthBarWidget.generated.h"

class UAbilitySystemComponent;
class UEDBaseAttributeSet;
class UProgressBar;
class UTextBlock;
struct FOnAttributeChangeData;

UCLASS()
class ETERNALDREAMS_API UEDFloatingHealthBarWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeDestruct() override;

	// 체력바가 추적할 ASC와 AttributeSet을 연결
	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void InitializeHealthSource(UAbilitySystemComponent* InAbilitySystemComponent, UEDBaseAttributeSet* InBaseAttributeSet);

protected:
	// 체력 바
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HealthBar")
	TObjectPtr<UProgressBar> HPBar;

	// 현재 체력 수치를 표시하는 텍스트
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HealthBar")
	TObjectPtr<UTextBlock> HPValueText;

private:
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<UEDBaseAttributeSet> CachedBaseAttributeSet;

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

	void BindHealthDelegates();
	void UnbindHealthDelegates();
	void RefreshHealthDisplay() const;

	void HandleHealthChanged(const FOnAttributeChangeData& Data) const;
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data) const;
};
