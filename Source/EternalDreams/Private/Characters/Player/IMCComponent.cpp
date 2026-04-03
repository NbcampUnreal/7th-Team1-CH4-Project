// Fill out your copyright notice in the Description page of Project Settings.


#include "IMCComponent.h"
#include "EnhancedInputComponent.h"
#include "SNegativeActionButton.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/EDPlayerController.h"


// Sets default values for this component's properties
UIMCComponent::UIMCComponent()
{

}


// Called when the game starts
void UIMCComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UIMCComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent) return;
	
	PlayerCharacter = Cast<AEDPlayerCharacter>(GetOwner());
	if (!PlayerCharacter) return;

	PlayerController = Cast<AEDPlayerController>(PlayerCharacter->GetController());
	if (!PlayerController) return;
	
	
	if (UEnhancedInputComponent* InputComponents = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (!IsValid(PlayerCharacter))
		{
			return;
		}
		if (IsValid(PlayerController))
		{
			// Move 바인딩
			InputComponents->BindAction(
				PlayerController->MoveAction,
				ETriggerEvent::Triggered,
				this,
				&UIMCComponent::PlayerMove
			);
			
			// Look 바인딩
			InputComponents->BindAction(
				PlayerController->LookAction,
				ETriggerEvent::Triggered,
				this,
				&UIMCComponent::PlayerLook
			);
		}
	}
}

void UIMCComponent::PlayerMove(const FInputActionValue& value)
{
	if (!PlayerController||!PlayerCharacter)
	{
		return;
	}
	const FVector2D MoveInput = value.Get<FVector2D>();
	
	PlayerCharacter->AddMovementInput(FVector(1,0,0), MoveInput.X);
	PlayerCharacter->AddMovementInput(FVector(0,1,0), MoveInput.Y);
	
}

void UIMCComponent::PlayerLook(const FInputActionValue& value)
{
	if (!PlayerController||!PlayerCharacter)
	{
		return;
	}
	
	FHitResult HitResult;
	if (PlayerController->GetHitResultUnderCursor(ECC_Visibility,false, HitResult))
	{
		FVector TargetLocation = HitResult.ImpactPoint;
		FVector StartLocation = PlayerCharacter->GetActorLocation();

		
		// 방향 Rotator 계산(Yaw만 사용)
		FRotator LookAtRotation = FRotationMatrix::MakeFromX(TargetLocation - StartLocation).Rotator();
		LookAtRotation.Pitch = 0.0f;
		LookAtRotation.Roll = 0.0f;
		
		if (IsValid(PlayerCharacter->GetMesh()))
		{
			PlayerCharacter->GetMesh()->SetWorldRotation(LookAtRotation+FRotator(0,-90.f,0));
		}

	}
}



