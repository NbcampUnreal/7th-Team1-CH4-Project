// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Base/GAS/ExecCalc_Damage.h"

#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Data/GameplayTag/EDGameplayTags.h"

struct FDamageStatics
{
	// Target의 Defensive를 Capture
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defensive);
	// Target의 Health를 Capture (최종 데미지 적용용)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);

	FDamageStatics()
	{
		// UMyAttributeSet의 Defensive, Target에서, Snapshot 안 함 (실시간 값)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UEDBaseAttributeSet, Defensive, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UEDBaseAttributeSet, Health, Target, false);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}


UExecCalc_Damage::UExecCalc_Damage()
{
	// Capture할 Attribute 등록
	RelevantAttributesToCapture.Add(DamageStatics().DefensiveDef);
	RelevantAttributesToCapture.Add(DamageStatics().HealthDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
    UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
    if (!SourceASC || !TargetASC) return;

    AActor* SourceActor = SourceASC->GetAvatarActor();
    AActor* TargetActor = TargetASC->GetAvatarActor();
    if (!SourceActor || !TargetActor)
    {
	    return;
    }

	
    float RawDamage = Spec.GetSetByCallerMagnitude(
        FEDGameplayTags::Get().Data_Damage,
        false,      
        30.0f       
    );

    float Defense = 0.f;

    FAggregatorEvaluateParameters EvalParams;
    EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().DefensiveDef,
        EvalParams,
        Defense
    );
	
	
    float FinalDamage = RawDamage - Defense;
    FinalDamage = FMath::Max(FinalDamage, 0.f);

	
    if (FinalDamage > 0.f)
    {
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(
                DamageStatics().HealthProperty,
                EGameplayModOp::Additive,
                -FinalDamage
            )
        );
    }
}
