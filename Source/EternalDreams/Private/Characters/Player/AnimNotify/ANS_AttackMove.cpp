// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AnimNotify/ANS_AttackMove.h"

#include "Characters/Player/EDPlayerCharacter.h"

void UANS_AttackMove::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp->GetOwner()->HasAuthority()==false)
	{
		return;
	}
	AActor* Owner = MeshComp->GetOwner();
	if (Owner == nullptr)
	{
		return;
	}
	AEDPlayerCharacter* Player = Cast<AEDPlayerCharacter>(Owner);
	if (Player == nullptr)
	{
		return;
	}
	Player->SetAnimRootMotionTranslationScale(0.f);
	Player->StartAnimMove(DashSpeed,bIsForwardDirection,bIsZDirection);
}



void UANS_AttackMove::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp->GetOwner()->HasAuthority()==false)
	{
		return;
	}
	AActor* Owner = MeshComp->GetOwner();
	AEDPlayerCharacter* Player = Cast<AEDPlayerCharacter>(Owner);
	if (Player == nullptr)
	{
		return;
	}
	Player->StopAnimMove();
}
