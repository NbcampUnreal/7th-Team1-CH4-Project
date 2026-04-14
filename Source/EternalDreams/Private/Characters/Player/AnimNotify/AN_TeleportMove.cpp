// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AnimNotify/AN_TeleportMove.h"

#include "Characters/Player/EDPlayerCharacter.h"

void UAN_TeleportMove::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AEDPlayerCharacter* Player=Cast<AEDPlayerCharacter>(MeshComp->GetOwner());
	if (!IsValid(Player))
	{
		return;
	}
	
	FVector TeleportLocation=Player->GetActorForwardVector()*TeleportDistance+Player->GetActorLocation();
	
	bool bCanTeleport = GetWorld()->OverlapAnyTestByChannel(
	TeleportLocation,
	Player->GetActorRotation().Quaternion(),
	ECollisionChannel::ECC_Pawn,
	FCollisionShape::MakeCapsule(48.f,128.f)
	);
	
	if (!bCanTeleport)
	{
		Player->SetActorLocation(TeleportLocation);
	}
	
}
