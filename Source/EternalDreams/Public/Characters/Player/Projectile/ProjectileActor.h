// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"


class UAbilitySystemComponent;
class IAbilitySystemInterface;
class UGameplayEffect;
class AMainGameMode;
class UProjectileMovementComponent;
class USphereComponent;
class AEnemyBase;
class AMainGameMode;
/**
 * 소환한 대상의 정면 방향으로 날아가며 Sphere Trace를 진행해, 맞으면 데미지를 입힙니다.
 */
UCLASS()
class ETERNALDREAMS_API AProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectileActor();
	

protected:
	virtual void BeginPlay() override;
	
	
public:
	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, 
		const FHitResult& Hit
		);
	
	
	void SetStaticMesh(UStaticMesh* StaticMesh);
	
	void LifeTimeEnd();
	
protected:
	//적용할 Damage Effect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> ProjectileStaticMesh;
	
private:
	UPROPERTY(EditAnywhere)
	float ProjectileSpeed=1000.0f;	
	
	UPROPERTY(VisibleAnywhere)
	bool bIsHoming=false;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement; 
	
	FTimerHandle TimerHandle;
	
	
};