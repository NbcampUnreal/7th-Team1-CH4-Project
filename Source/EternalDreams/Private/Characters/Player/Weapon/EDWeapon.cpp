// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Weapon/EDWeapon.h"


// Sets default values
AEDWeapon::AEDWeapon()
{
	Scene=CreateDefaultSubobject<USceneComponent>("Scene");
	SetRootComponent(Scene);
	WeaponStaticMesh=CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	WeaponStaticMesh->SetupAttachment(Scene);
	WeaponStaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
}

