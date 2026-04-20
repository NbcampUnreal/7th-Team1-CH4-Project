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
	
	UFUNCTION(BlueprintCallable,Server, Reliable)
	void SetStaticMesh(UStaticMesh* StaticMesh);
	
	UFUNCTION()
	FORCEINLINE void SetServerStaticMesh(UStaticMesh* StaticMesh){WeaponStaticMeshComp->SetStaticMesh(StaticMesh);};
	
	void ApplyMeshOnServer(UStaticMesh* StaticMesh);
	
	UFUNCTION()
	void OnRep_WeaponStaticMesh();
	
	UFUNCTION()
	FORCEINLINE UStaticMesh* GetStaticMesh() {return WeaponStaticMeshComp->GetStaticMesh();};
	
	FORCEINLINE UStaticMeshComponent* GetStaticMeshComp() {return WeaponStaticMeshComp;};

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyMesh(UStaticMesh* StaticMesh);
	
protected:
	UPROPERTY()
	TObjectPtr<USceneComponent> Scene;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> WeaponStaticMeshComp;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Static Mesh",ReplicatedUsing=OnRep_WeaponStaticMesh)
	TObjectPtr<UStaticMesh> WeaponStaticMesh;
	
	
	
};
