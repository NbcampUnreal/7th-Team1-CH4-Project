// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "EDLightingDataStruct.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FEDLightingDataStruct : public FTableRowBase
{
	GENERATED_BODY()

	public:
	// 디렉셔널 라이트 강도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float DLLightIntensity;

	// 색온도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float DLColorTemperature;

	// 포스트 프로세스 노출(Exposure)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float PPVExposureBias;
    
	// 기본값 초기화 (생성자)
	FEDLightingDataStruct()
		: DLLightIntensity(10.0f)
		, DLColorTemperature(6500.0f)
		, PPVExposureBias(11.0f)
	{}
};
