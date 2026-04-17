// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Core/EDGameDataSubsystem.h"
#include "Data/Types/EDMonsterTypes.h"
#include "EDMonsterBase.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnMonsterDeath);
DECLARE_MULTICAST_DELEGATE(FOnAttackFinished);

class UAbilitySystemComponent;
class UEDMonsterDataAsset;
struct FStreamableHandle;

UCLASS()
class ETERNALDREAMS_API AEDMonsterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEDMonsterBase();
	
	virtual  UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	
	UFUNCTION(BlueprintCallable, Category = "Data")
	void InitializeFromDataAsset(UEDMonsterDataAsset* InDataAsset);
	
	// 수정 전
	// UFUNCTION(BlueprintCallable, Category= "Data")
	// UEDMonsterDataAsset* GetDataAsset() const { return DataAsset; }
	// 수정 후
	UFUNCTION(BlueprintCallable, Category = "Data")
	UEDMonsterDataAsset* GetDataAsset() const 
	{
		// 캐싱 사용
		if (UEDGameDataSubsystem* Subsystem = UEDGameDataSubsystem::Get(this))
		{
			return Subsystem->GetData<UEDMonsterDataAsset>(MonsterDataId);
		}
		return nullptr;
	}
	
	FVector GetOriginLocation() const { return OriginLocation;}
	
	FOnMonsterDeath OnMonsterDeath;
	FOnAttackFinished OnAttackFinished;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_MonsterState();	
	
	UFUNCTION(BlueprintCallable, Category = "Monster")
	void SetMonsterState(EMonsterState NewState) { if (HasAuthority()) MonsterState = NewState; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	// 수정 전
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	// TObjectPtr<UEDMonsterDataAsset> DataAsset;
	// 수정 후
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Data")
	FPrimaryAssetId MonsterDataId;
private:
	// 비동기 로드(임시)
	void LoadVisuals(UEDMonsterDataAsset* InDataAsset);
	// 비동기 로드 완료 콜백
	void OnVisualsLoaded();
	
	// Health 가 0 이하가 됐을때 호출
	void HandleDeath();
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	UPROPERTY(ReplicatedUsing = OnRep_MonsterState)
	EMonsterState MonsterState;
	
	UPROPERTY()
	TObjectPtr<UEDBaseAttributeSet> BaseAttributeSet;
	
	FVector OriginLocation;
	TSharedPtr<FStreamableHandle> VisualLoadHandle;
	
	TWeakObjectPtr<UEDGameDataSubsystem> CachedDataSubsystem;
	FTimerHandle DestroyMeshTimerHandle;
};
