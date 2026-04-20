// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AnimNotify/AN_ProjectileAttack.h"

#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/Projectile/ProjectileActor.h"
#include "Characters/Player/Weapon/EDWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


void UAN_ProjectileAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (MeshComp==nullptr||MeshComp->GetWorld()==nullptr)
	{
		return;
	}
	if (MeshComp->GetOwner()->HasAuthority()==false)
	{
		return;
	}
	
	AEDPlayerCharacter* Player=Cast<AEDPlayerCharacter>(MeshComp->GetOwner());
	if (Player==nullptr)
	{
		return;
	}
	
	if (Player->GetMesh()!=MeshComp)
	{
		return;
	}
	
	
	UStaticMeshComponent* WeaponMesh=Player->GetWeaponMeshComp();
	if (WeaponMesh==nullptr)
	{
		return;
	}
	
	//발사체의 위치와 방향을 세팅합니다.
	FVector SpawnLocation = WeaponMesh->GetSocketTransform(SocketName, RTS_World).GetLocation()+MeshComp->GetOwner()->GetActorForwardVector()*50.f;
	
	//기존 바라보는 방향대로 타겟
	FRotator SpawnRotation=MeshComp->GetOwner()->GetActorRotation();
	
	
	
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	AActor* Projectile=MeshComp->GetWorld()->SpawnActorDeferred<AProjectileActor>(ProjectileClass,SpawnTransform);
	
	if (IsValid(Projectile))
	{
		Projectile->SetOwner(MeshComp->GetOwner());
		Projectile->FinishSpawning(SpawnTransform);
	}
	
}