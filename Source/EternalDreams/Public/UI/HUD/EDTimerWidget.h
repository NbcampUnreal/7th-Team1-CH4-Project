// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EDTimerWidget.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDTimerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "UI|Timer")
	void UpdateTimeText(float InRemainingTime);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TimeText;
};
