// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "EDAssetManager.generated.h"

/**
 * 
 */
UCLASS(Config = Game)
class ETERNALDREAMS_API UEDAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	UEDAssetManager();
	
	/**
	 * 전역 UEDAssetManager 싱글톤 인스턴스 반환
	 * ex) UEDAssetManager& Manager = UEDAssetManager::Get();
	 */ 
	static UEDAssetManager& Get();

	/**
	 * 엔진 시작시 에셋 매니저 초기화할때 자동 호출
	 *  
	 */
	virtual void StartInitialLoading() override;
};
