// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDCameraActor.generated.h"

class AEDPlayerCharacter;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class ETERNALDREAMS_API AEDCameraActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEDCameraActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//Camera Components
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpringArmLength=1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraOffset=FVector(-20.0f,0,20.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FRotator CameraRotation=FRotator(-40.f,0,0);
	
	//Bool Attributes
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	bool bIsFocusedPlayer=true;
	
	//Caching
protected:
	UPROPERTY()
	TObjectPtr<AEDPlayerCharacter> PlayerCharacter;
};
