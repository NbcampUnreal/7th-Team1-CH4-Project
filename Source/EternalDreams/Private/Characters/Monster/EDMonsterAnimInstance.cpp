// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/EDMonsterAnimInstance.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEDMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	CurrentState = EMonsterState::Idle;
	MoveSpeed = 0.f;
}

void UEDMonsterAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(TryGetPawnOwner());
	if (IsValid(Monster) == false)
		return;
	
	MoveSpeed = Monster->GetCharacterMovement()->Velocity.Size();
}

void UEDMonsterAnimInstance::SetMonsterState(EMonsterState NewState)
{
	CurrentState = NewState;
	UE_LOG(LogTemp, Warning, TEXT("[%s] AnimInstance State %d"), *GetOwningActor()->GetName(), (int32)CurrentState);
}
