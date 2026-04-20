// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/EDLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Engine/PostProcessVolume.h"


// Sets default values
AEDLightingManager::AEDLightingManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEDLightingManager::StartTransition(FName PhaseRowName, float Duration)
{
	if (!LightingDataTable) return;

	FEDLightingDataStruct* Row = LightingDataTable->FindRow<FEDLightingDataStruct>(
		PhaseRowName, TEXT("Environment Update"));

	if (Row)
	{
		// 현재 상태를 시작점으로 저장
		StartSettings = TargetSettings;
		TargetSettings = *Row;

		TransitionDuration = Duration;
		TransitionElapsed = 0.0f;
		bIsTransitioning = true;
	}
}

void AEDLightingManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsTransitioning) return;

	TransitionElapsed += DeltaTime;
	float Alpha = FMath::Clamp(TransitionElapsed / TransitionDuration, 0.0f, 1.0f);

	// Ease-InOut 효과를 주어 더 자연스럽게 만듭니다.
	float InterpAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	FEDLightingDataStruct InterpSettings;
	InterpSettings.DLLightIntensity = FMath::Lerp(StartSettings.DLLightIntensity, TargetSettings.DLLightIntensity,
	                                              InterpAlpha);
	InterpSettings.DLColorTemperature = FMath::Lerp(StartSettings.DLColorTemperature, TargetSettings.DLColorTemperature,
	                                                InterpAlpha);
	InterpSettings.PPVExposureBias = FMath::Lerp(StartSettings.PPVExposureBias, TargetSettings.PPVExposureBias,
	                                             InterpAlpha);

	ApplySettings(InterpSettings);

	// 전환 완료
	if (Alpha >= 1.0f)
	{
		bIsTransitioning = false;
	}
}

void AEDLightingManager::ApplySettings(const FEDLightingDataStruct& Settings)
{
	if (TargetLight)
	{
		TargetLight->GetLightComponent()->SetIntensity(Settings.DLLightIntensity);
		TargetLight->GetLightComponent()->SetTemperature(Settings.DLColorTemperature);
	}
	if (TargetPPV)
	{
		TargetPPV->Settings.bOverride_AutoExposureBias = true;
		TargetPPV->Settings.AutoExposureBias = Settings.PPVExposureBias;
	}
}
