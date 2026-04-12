// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "ZoneDetectorComponent.generated.h"

class UGameplayEffect;
class UEDTimerWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ETERNALDREAMS_API UZoneDetectorComponent : public UActorComponent
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
	
public:	
	UZoneDetectorComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override; // 추가됨

	// 구역 액터가 서버에서 호출해 줄 함수들
	void EnterRestrictedArea(TSubclassOf<UGameplayEffect> EffectClass);
	void ExitRestrictedArea();

protected:
	// 서버가 특정 클라이언트(나)에게만 실행하라고 명령하는 RPC 함수
	UFUNCTION(Client, Reliable)
	void Client_ShowZoneDebugMessage(bool bIsEntering, int32 CurrentZoneCount);
	
protected:
	// 콜백함수
	// virtual void OnRestrictedAreaTagChanged(const FGameplayTag Tag, int32 NewCount);

public:
	// 타이머 위젯 클래스 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UEDTimerWidget> TimerWidgetClass;
	
private:
	int32 OverlappingZoneCount = 0;
	FActiveGameplayEffectHandle RestrictedAreaEffectHandle;
	
	UPROPERTY()
	UEDTimerWidget* TimerWidget;
};