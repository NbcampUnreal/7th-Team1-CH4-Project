// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_AttackTrace.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class IAbilitySystemInterface;
class AEDPlayerCharacter;
class AEDWeapon;
/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UANS_AttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()
	UANS_AttackTrace();

	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Enemy|Attack")
	float TraceRadius=50.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Enemy|Attack")
	bool bShowDebug;
	
	//XY방향으로 날리는 힘
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Enemy|Attack")
	float LaunchPowerXY=0.f;
	//Z방향으로 날리는 힘
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Enemy|Attack")
	float LaunchPowerZ=0.f;
	
	//적용할 Damage Effect 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
private:
	FVector PresentAttackSocketLocation=FVector::ZeroVector;
	FVector CurrentAttackSocketLocation=FVector::ZeroVector;
	//공격을 진행하는 액터
	UPROPERTY()
	TObjectPtr<AActor> Owner;
	
	
	//Weapon Actor
	UPROPERTY()
	TObjectPtr<AEDWeapon> Weapon=nullptr;
	//Weapon Actor
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	//맞은 액터
	UPROPERTY()
	TObjectPtr<AActor> HittedActor;
	//형변환 캐싱
	UPROPERTY()
	TObjectPtr<AEDPlayerCharacter> HittedPlayer;
	//Weapon을 가져오기 위한 Array
	UPROPERTY()
	TArray<AActor*> AttachedActors;
	//중복 타격 방지를 위해 TArray에 저장
	UPROPERTY()	
	TArray<AActor*> HittedCharacterArray;
	//소켓이름
	FName SocketName=FName("Socket");
	//공격자 ASI 캐싱
	TObjectPtr<IAbilitySystemInterface> AttackerASI;
	//공격자 ASC 캐싱
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AttackerASC;

	
	
	
	
};
