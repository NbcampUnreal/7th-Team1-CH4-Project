// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_HitScanAttack.generated.h"

class UAbilitySystemComponent;
class IAbilitySystemInterface;
class AEDPlayerCharacter;
class AEDWeapon;
class UGameplayEffect;
/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UANS_HitScanAttack : public UAnimNotifyState
{
	GENERATED_BODY()
	UANS_HitScanAttack();
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<AActor> WarningActorClass=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<AActor> FinalShootActorClass=nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float AttackDistance;
	
	//적용할 Damage Effect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float ShootActorLifeSpan=2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bShowDebug=false;
	
private:

	//공격을 진행하는 액터
	UPROPERTY()
	TObjectPtr<AActor> Owner;
	
	//Weapon Actor
	UPROPERTY()
	TObjectPtr<AEDWeapon> Weapon=nullptr;
	//Weapon Actor
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	//소켓이름
	FName SocketName=FName("Socket");
	//공격자 ASI 캐싱
	IAbilitySystemInterface* AttackerASI;
	//공격자 ASC 캐싱
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AttackerASC;
	
	FVector SocketLocation=FVector::ZeroVector;
	FVector SocketDirection=FVector::ZeroVector;
	
	FTransform SpawnTransform;
	
	
};
