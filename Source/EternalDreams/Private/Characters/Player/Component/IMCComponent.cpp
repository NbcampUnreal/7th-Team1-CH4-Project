// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/IMCComponent.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "SNegativeActionButton.h"
#include "ToolBuilderUtil.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/EDPlayerController.h"
#include "Components/WidgetComponent.h"
#include "Data/GameplayTag/EDGameplayTags.h"




// Called when the game starts
void UIMCComponent::BeginPlay()
{
	Super::BeginPlay();
	

}


void UIMCComponent::SetupPlayerInput(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	PlayerCharacter = Cast<AEDPlayerCharacter>(GetOwner());
	if (!EnhancedInputComponent||!PlayerCharacter)
	{
		return;
	}
	
	//Tag 및 바인딩
	PlayerCharacter->GetAbilitySystemComponent()->
	RegisterGameplayTagEvent(FEDGameplayTags::Get().State_Player_Stop,EGameplayTagEventType::NewOrRemoved).
	AddUObject(this,&UIMCComponent::OnStopTagChanged);
	
	PlayerController = Cast<AEDPlayerController>(PlayerCharacter->GetController());
	if (!PlayerController)
	{
		return;
	}
	
	
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
			
			// BasicAttack 바인딩
			InputComponents->BindAction(
				PlayerController->BasicAttackAction,
				ETriggerEvent::Started,
				this,
				&UIMCComponent::PlayerBasicAttack
			);
			
			// QSkill 바인딩
			InputComponents->BindAction(
				PlayerController->QSkillAction,
				ETriggerEvent::Started,
				this,
				&UIMCComponent::PlayerQSkill
			);
			
			// ESkill 바인딩
			InputComponents->BindAction(
				PlayerController->ESkillAction,
				ETriggerEvent::Started,
				this,
				&UIMCComponent::PlayerESkill
			);
			
			// SpaceSkill 바인딩
			InputComponents->BindAction(
				PlayerController->SpaceSkillAction,
				ETriggerEvent::Started,
				this,
				&UIMCComponent::PlayerSpaceSkill
			);
		}
	}
}

void UIMCComponent::OnStopTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount>0)
	{
		bIsStop=true;
	}
	else
	{
		bIsStop=false;
	}
}

void UIMCComponent::PlayerMove(const FInputActionValue& value)
{
	if (!PlayerController||!PlayerCharacter||bIsStop)
	{
		return;
	}
	const FVector2D MoveInput = value.Get<FVector2D>();
	
	PlayerCharacter->AddMovementInput(FVector(1,0,0), MoveInput.X);
	PlayerCharacter->AddMovementInput(FVector(0,1,0), MoveInput.Y);
}

void UIMCComponent::PlayerLook(const FInputActionValue& value)
{
	if (!PlayerController||!PlayerCharacter||bIsStop)
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
		
		PlayerController->SetControlRotation(LookAtRotation);
		
	}
	
}

void UIMCComponent::PlayerBasicAttack(const FInputActionValue& value)
{
	OnBasicAttackInput.ExecuteIfBound();
}

void UIMCComponent::PlayerQSkill(const FInputActionValue& value)
{
	OnQSkillInput.ExecuteIfBound();
}

void UIMCComponent::PlayerESkill(const FInputActionValue& value)
{
	OnESkillInput.ExecuteIfBound();
}

void UIMCComponent::PlayerSpaceSkill(const FInputActionValue& value)
{
	OnSpaceSkillInput.ExecuteIfBound();
}


