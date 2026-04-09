// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h" 
#include "ZoneDetectorComponent.generated.h"

class UGameplayEffect;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ETERNALDREAMS_API UZoneDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UZoneDetectorComponent();

	// 구역 액터가 서버에서 호출해 줄 함수들
	void EnterRestrictedArea(TSubclassOf<UGameplayEffect> EffectClass);
	void ExitRestrictedArea();

protected:
	// 서버가 특정 클라이언트(나)에게만 실행하라고 명령하는 RPC 함수
	UFUNCTION(Client, Reliable)
	void Client_ShowZoneDebugMessage(bool bIsEntering, int32 CurrentZoneCount);

private:
	int32 OverlappingZoneCount = 0;
	FActiveGameplayEffectHandle RestrictedAreaEffectHandle;
};