// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ProjectileAttack.generated.h"

/**
 * 해당 노티파이 시점에서 발사체를 스폰합니다.
 */
class AProjectileActor;
UCLASS()
class ETERNALDREAMS_API UAN_ProjectileAttack : public UAnimNotify
{
	GENERATED_BODY()
public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
							const FAnimNotifyEventReference& EventReference)override;

protected:
	FName SocketName=FName("Socket");
	
	
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectileActor> ProjectileClass;
	
	TObjectPtr<AProjectileActor> Projectile;
	
	TArray<AActor*> AttachedActors;
};