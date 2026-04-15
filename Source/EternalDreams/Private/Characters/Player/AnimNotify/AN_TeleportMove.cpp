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
	
	FVector TeleportLocation=Player->GetActorForwardVector()*TeleportDistance+Player->GetActorLocation()+FVector(0,0,5.f);
	
	bool bCanTeleport =MeshComp->GetWorld()->OverlapAnyTestByProfile(
	TeleportLocation,
	Player->GetActorRotation().Quaternion(),
	FName("Visibility"),
	FCollisionShape::MakeCapsule(48.f, 128.f)
	);
	DrawDebugCapsule(MeshComp->GetWorld(), TeleportLocation, 128.f, 48.f,Player->GetActorRotation().Quaternion(), 
	bCanTeleport ? FColor::Red : FColor::Green, false, 3.0f, 0, 2.0f);
	
	
	if (!bCanTeleport)
	{
		Player->SetActorLocation(TeleportLocation);
	}
	
}
