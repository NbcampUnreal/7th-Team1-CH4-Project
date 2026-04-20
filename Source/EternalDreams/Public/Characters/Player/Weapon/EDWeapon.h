// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDWeapon.generated.h"

UCLASS()
class ETERNALDREAMS_API AEDWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEDWeapon();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	FORCEINLINE UStaticMesh* GetStaticMesh() {return WeaponStaticMeshComp->GetStaticMesh();};
	
	FORCEINLINE UStaticMeshComponent* GetStaticMeshComp() {return WeaponStaticMeshComp;};
	
	UFUNCTION()
	void SetStaticMeshId(const FPrimaryAssetId& InStaticMeshId){if (!HasAuthority()){return;} StaticMeshId=InStaticMeshId; ApplyWeaponMesh();};
	
	UFUNCTION()
	FORCEINLINE void OnRep_StaticMeshId(){ApplyWeaponMesh();}
	
protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> Scene;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> WeaponStaticMeshComp;
	
	UPROPERTY(ReplicatedUsing = OnRep_StaticMeshId)
	FPrimaryAssetId StaticMeshId;
	
	UFUNCTION()
	void ApplyWeaponMesh();
	
};
