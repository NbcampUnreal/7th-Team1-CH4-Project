// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_TeleportMove.generated.h"

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UAN_TeleportMove : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify ( USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
	
protected:
	UPROPERTY(EditAnywhere,Category="Distance")
	float TeleportDistance=100.f;
	
};
