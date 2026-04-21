// Fill out your copyright notice in the Description page of Project Settings.


#include "Environment/EDLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Engine/PostProcessVolume.h"


// Sets default values
AEDLightingManager::AEDLightingManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; 
	bAlwaysRelevant = true; // 맵 어디에 있든 항상 통신을 받도록 설정
}

void AEDLightingManager::Multicast_StartTransition_Implementation(FName PhaseRowName, float Duration)
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

	//Duration이 0이 되지 않도록 보정
	TransitionElapsed += DeltaTime;
	float SafeDuration = FMath::Max(TransitionDuration, 0.001f);
	float Alpha = FMath::Clamp(TransitionElapsed / SafeDuration, 0.0f, 1.0f);

	// Ease-InOut 효과를 주어 더 자연스럽게 만듭니다.
	float InterpAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	FEDLightingDataStruct InterpSettings;
    
	// FMath::Lerp를 사용하여 시작점부터 끝점까지 부드럽게 계산
	InterpSettings.DLLightIntensity = FMath::Lerp(StartSettings.DLLightIntensity, TargetSettings.DLLightIntensity, InterpAlpha);
	InterpSettings.DLColorTemperature = FMath::Lerp(StartSettings.DLColorTemperature, TargetSettings.DLColorTemperature, InterpAlpha);
	InterpSettings.PPVExposureBias = FMath::Lerp(StartSettings.PPVExposureBias, TargetSettings.PPVExposureBias, InterpAlpha);

	ApplySettings(InterpSettings);

	if (Alpha >= 1.0f)
	{
		bIsTransitioning = false;
		// 마지막에 타겟 수치를 정확히 꽂아줍니다.
		ApplySettings(TargetSettings); 
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
