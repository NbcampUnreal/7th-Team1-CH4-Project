// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/EDMonsterTypes.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Abilities/GameplayAbility.h"
#include "EDMonsterDataAsset.generated.h"

class AEDMonsterBase;

/**
 * 
 */
UCLASS()
class ETERNALDREAMS_API UEDMonsterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// getter함수에서 const 자료형& 변수명 const{return 값;} 을 쓰려면 BlueprintCallable을 사용 불가 
	// 테스트또는 UI 등에서 데이터 에셋 값을 블루프린트를 이용하여 불러올것을 대비해서 값복사 방식으로 쓰고 BlueprintCallable 유지
	UFUNCTION(BlueprintCallable)
	FText GetMonsterName() const { return MonsterName; }
	
	UFUNCTION(BlueprintCallable)
	EMonsterGrade GetGrade() const { return Grade; }
	
	UFUNCTION(BlueprintCallable)
	FMonsterStatRow GetStat() const { return StatData; }
	
	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<USkeletalMesh> GetMesh() const { return Mesh; }
	
	UFUNCTION(BlueprintCallable)
	TSoftClassPtr<UAnimInstance> GetAnimInstance() const { return AnimInstance; }
	
	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<UBehaviorTree> GetBehaviorTree() const { return BehaviorTree; }
	
	UFUNCTION(BlueprintCallable)
	virtual FPrimaryAssetId GetPrimaryAssetId() const override { return FPrimaryAssetId(TEXT("MonsterData"), GetFName()); }
	
	UFUNCTION(BlueprintCallable)
	TArray<TSubclassOf<UGameplayAbility>> GetDefaultAbilities() const { return DefaultAbilities; }
	
	UFUNCTION(BlueprintCallable)
	TSubclassOf<AEDMonsterBase> GetMonsterClass() const { return MonsterClass; }
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	FText MonsterName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	EMonsterGrade Grade;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	FMonsterStatRow StatData;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TSoftObjectPtr<USkeletalMesh> Mesh;
	// 임시 -> 추후 EDMonsterAnimInstance로 교체 예정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TSoftClassPtr<UAnimInstance> AnimInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;
	// TODO
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	TSubclassOf<AEDMonsterBase> MonsterClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
};
