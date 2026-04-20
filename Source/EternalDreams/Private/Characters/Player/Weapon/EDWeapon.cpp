// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Weapon/EDWeapon.h"

#include "Core/EDGameDataSubsystem.h"
#include "Data/EDWeaponDataAsset.h"
#include "Data/Types/EDPlayerTypes.h"
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

void AEDWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AEDWeapon,StaticMeshId);
}


void AEDWeapon::SetStaticMeshId(const FPrimaryAssetId& InStaticMeshId)
{
	if (HasAuthority())
	{
		StaticMeshId=InStaticMeshId;
		ApplyWeaponMesh();
	}
}

void AEDWeapon::OnRep_StaticMeshId()
{
	ApplyWeaponMesh();
}

void AEDWeapon::ApplyWeaponMesh()
{
	if (!StaticMeshId.IsValid())
	{
		WeaponStaticMeshComp->SetStaticMesh(nullptr);
	}
	
	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (!EDGameplayDataSubsystem)
	{
		return;
	}
	
	UEDWeaponDataAsset* Weapon = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(StaticMeshId
				);
		
	if (IsValid(Weapon))
	{
		WeaponStaticMeshComp->SetStaticMesh(Weapon->WeaponStaticMesh.LoadSynchronous());
	}
}



