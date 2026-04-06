// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDCursorActor.generated.h"

class UWidgetComponent;

UCLASS()
class ETERNALDREAMS_API AEDCursorActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEDCursorActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//커서 UI
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<USceneComponent> Scene;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TObjectPtr<UWidgetComponent> CursorWidget;
	
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;
};
