// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/Types/EDMonsterTypes.h"
#include "EDMonsterAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class ETERNALDREAMS_API UEDMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(const float DeltaSeconds) override;
	
	UFUNCTION(BlueprintCallable, Category = "Anim|State")
	void SetMonsterState(EMonsterState NewState);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	EMonsterState CurrentState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anim")
	float MoveSpeed;
};
