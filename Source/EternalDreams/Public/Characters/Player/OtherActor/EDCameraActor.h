// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDCameraActor.generated.h"

class AEDPlayerController;
class AEDPlayerCharacter;
class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

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
	/*
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpringArmLength=1000.f;
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraOffset=FVector::ZeroVector;
	//최소로 줌인한 카메라 오프셋
	UPROPERTY()
	FVector MinZoomCameraOffset;
	//최대로 줌인한 카메라 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector MaxZoomCameraOffset=FVector(-180.f,0,150.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FRotator CameraRotation=FRotator::ZeroRotator;
	
	//Control Attributes
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	bool bIsFocusedPlayer=true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float CameraScrollSpeed=10.0f;
	
	
	//Caching
protected:
	UPROPERTY()
	TObjectPtr<AEDPlayerCharacter> PlayerCharacter;
	UPROPERTY()
	TObjectPtr<AEDPlayerController> PlayerController;
	UPROPERTY()
	FVector CameraFrontVector;
	UPROPERTY()
	FVector BufferVector;
	
	//CallBack
public:
	UFUNCTION()
	void CameraZoom(FInputActionValue value);
	UFUNCTION()
	void ToggleCameraFocus(FInputActionValue value);
	UFUNCTION()
	void CameraMove(FInputActionValue value);
};
