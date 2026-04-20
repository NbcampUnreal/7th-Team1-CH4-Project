// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Weapon/EDWeapon.h"

#include "Net/UnrealNetwork.h"


// Sets default values
AEDWeapon::AEDWeapon()
{
	bReplicates=true;
	
	Scene=CreateDefaultSubobject<USceneComponent>("Scene");
	SetRootComponent(Scene);
	WeaponStaticMeshComp=CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	WeaponStaticMeshComp->SetupAttachment(Scene);
	WeaponStaticMeshComp->SetCollisionProfileName(TEXT("NoCollision"));
}


void AEDWeapon::ApplyMeshOnServer(UStaticMesh* StaticMesh)
{
	if (!HasAuthority()) return;
	WeaponStaticMesh = StaticMesh;
	MulticastApplyMesh(StaticMesh);
}

void AEDWeapon::OnRep_WeaponStaticMesh()
{
	WeaponStaticMeshComp->SetStaticMesh(WeaponStaticMesh);
	UE_LOG(LogTemp,Warning,TEXT("Rep"));
}

void AEDWeapon::MulticastApplyMesh_Implementation(UStaticMesh* StaticMesh)
{
	WeaponStaticMeshComp->SetStaticMesh(StaticMesh);
}

void AEDWeapon::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AEDWeapon,WeaponStaticMesh);
}

void AEDWeapon::SetStaticMesh_Implementation(UStaticMesh* StaticMesh)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp,Warning,TEXT("Authority"));
		WeaponStaticMesh = StaticMesh;
		
		OnRep_WeaponStaticMesh(); 
	}
}

