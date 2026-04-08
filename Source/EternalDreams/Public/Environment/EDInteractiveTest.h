// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDInteractiveTest.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class ETERNALDREAMS_API AEDInteractiveTest : public AActor
{
	GENERATED_BODY()

public:	
	AEDInteractiveTest();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;
};
