// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/EDLightingDataStruct.h" 
#include "EDLightingManager.generated.h"

class UDataTable;
class ADirectionalLight;
class APostProcessVolume;

UCLASS()
class ETERNALDREAMS_API AEDLightingManager : public AActor
{
	GENERATED_BODY()

public:
	AEDLightingManager();
	
	virtual void Tick(float DeltaTime) override; 

	// 페이즈 전환 시 호출할 함수
	void StartTransition(FName PhaseRowName, float Duration = 5.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	UDataTable* LightingDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	ADirectionalLight* TargetLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	APostProcessVolume* TargetPPV;

protected:
	// 상태 관리 변수
	bool bIsTransitioning = false;
	float TransitionElapsed = 0.0f;
	float TransitionDuration = 5.0f;

	// 라이팅 보간을 위한 데이터 저장
	FEDLightingDataStruct StartSettings;
	FEDLightingDataStruct TargetSettings;

	// 실제 값 적용 함수
	void ApplySettings(const FEDLightingDataStruct& Settings);
};
