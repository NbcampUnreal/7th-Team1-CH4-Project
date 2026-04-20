// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AnimNotify/ANS_HitScanAttack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/Weapon/EDWeapon.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"

UANS_HitScanAttack::UANS_HitScanAttack()
{
}

void UANS_HitScanAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp->GetOwner()->HasAuthority()==false)
	{
		return;
	}
	
	AEDPlayerCharacter* Player=Cast<AEDPlayerCharacter>(MeshComp->GetOwner());
	if (!IsValid(Player)||Player->GetWeaponMeshComp()==nullptr)
	{
		return;
	}
	//발사체의 위치와 방향을 세팅합니다.
	Player->SocketLocation=Player->GetWeaponMeshComp()->GetSocketTransform(SocketName, RTS_World).GetLocation();
	Player->SocketDirection=Player->GetActorForwardVector();
	
	
	FVector SpawnLocation = Player->SocketLocation+Player->SocketDirection*(AttackDistance/2);
	
	//기존 바라보는 방향대로 타겟
	FRotator SpawnRotation=Player->GetActorRotation();
	
	Player->SpawnTransform.SetLocation(SpawnLocation);
	Player->SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	AActor* WarningActor=MeshComp->GetWorld()->SpawnActorDeferred<AActor>(WarningActorClass,Player->SpawnTransform);
	
	if (IsValid(WarningActor))
	{
		WarningActor->SetOwner(MeshComp->GetOwner());
		WarningActor->FinishSpawning(Player->SpawnTransform);
		WarningActor->SetLifeSpan(TotalDuration);
		WarningActor->SetActorScale3D(FVector(1.f,1.f,AttackDistance));
		UE_LOG(LogTemp,Warning,TEXT("WarningActor Spawn"));
	}
	
	
}

void UANS_HitScanAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (MeshComp->GetOwner()->HasAuthority()==false)
	{
		return;
	}
	
	AEDPlayerCharacter* Player=Cast<AEDPlayerCharacter>(MeshComp->GetOwner());
	if (!IsValid(Player)||Player->GetWeaponMeshComp()==nullptr)
	{
		return;
	}
	
	
	if (!FinalShootActorClass)
	{
		return;
	}
	AActor* ShootActor=MeshComp->GetWorld()->SpawnActorDeferred<AActor>(FinalShootActorClass,Player->SpawnTransform);
	
	if (IsValid(ShootActor))
	{
		ShootActor->SetOwner(MeshComp->GetOwner());
		ShootActor->SetLifeSpan(ShootActorLifeSpan);
		ShootActor->FinishSpawning(Player->SpawnTransform);
		ShootActor->SetActorScale3D(FVector(1.f,1.f,AttackDistance));
		UE_LOG(LogTemp,Warning,TEXT("ShootActor Spawn"));
	}
	
	//Sphere Trace
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	FVector StartLocation= Player->SocketLocation;
	FVector EndLocation=StartLocation+Player->SocketDirection*AttackDistance;
	
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		Player->GetWorld(),
		StartLocation,
		EndLocation,
		//Pawn만 Trace 처리
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, // bTraceComplex
		ActorsToIgnore,
		DebugType,
		HitResult,
		true, // bIgnoreSelf
		FLinearColor::Red, // TraceColor
		FLinearColor::Green, // TraceHitColor
		2.0f // DrawTime
	);
	
	if (!bHit)
	{
		return;	
	}
	
	AActor* HittedActor=HitResult.GetActor();

	if (!HittedActor)
	{
		return;
	}


	FGameplayEventData HitGameplayEventData;
    		
	HitGameplayEventData.Target=HittedActor;
	Player->GetAbilitySystemComponent()->HandleGameplayEvent(FEDGameplayTags::Get().Event_SkillHit,&HitGameplayEventData);
		


}
