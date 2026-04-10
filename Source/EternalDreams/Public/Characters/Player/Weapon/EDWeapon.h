// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDWeapon.generated.h"

UCLASS()
class ETERNALDREAMS_API AEDWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEDWeapon();

protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> Scene;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Static Mesh")
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;
};
