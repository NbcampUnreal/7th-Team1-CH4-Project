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
	float AttackDistance=1000.f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float ShootActorLifeSpan=2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bShowDebug=false;
	
private:
	
	//소켓이름
	FName SocketName=FName("Socket");

	
	
	
};
