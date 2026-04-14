// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_AttackMove.generated.h"

class AEDPlayerCharacter;
/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UANS_AttackMove : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Speed")
	float DashSpeed=2.0f;
	
	//디폴트 : Forward로 대시하는 경우
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Direction")
	bool bIsForwardDirection=true;
	//Z방향으로 대시하는 경우
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Direction")
	bool bIsZDirection=false;
	
	UPROPERTY()
	FVector MoveVector;
	

	
private:
	UPROPERTY()
	TObjectPtr<AActor> Owner;
	//Player 형변환 후 캐싱
	UPROPERTY()
	TObjectPtr<AEDPlayerCharacter> Player;

	
};
