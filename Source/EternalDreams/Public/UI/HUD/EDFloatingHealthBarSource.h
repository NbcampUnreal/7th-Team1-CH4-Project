#pragma once

class UAbilitySystemComponent;
class UEDBaseAttributeSet;

DECLARE_MULTICAST_DELEGATE_TwoParams(FEDOnFloatingHealthBarSourceChanged, UAbilitySystemComponent*, UEDBaseAttributeSet*);
