// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_MonsterAttackTrace.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class AEDMonsterBase;

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UANS_MonsterAttackTrace : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UANS_MonsterAttackTrace();
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Attack")
	float TraceRadius = 50.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Attack")
	bool bShowDebug = false;
	
	// 몬스터 공격 부위 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Attack")
	FName SocketName = FName("hand_r");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
private:
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FVector> PrevLocationMap;
	//FVector CurrSocketLocation = FVector::ZeroVector;
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TArray<TWeakObjectPtr<AActor>>> HitMap;
	
	// UPROPERTY()
	// TObjectPtr<AEDMonsterBase> Monster;
	// UPROPERTY()
	// TObjectPtr<UAbilitySystemComponent> MonsterASC;
	
	//UPROPERTY()
	//TArray<AActor*> HitCharacterArray;
};
