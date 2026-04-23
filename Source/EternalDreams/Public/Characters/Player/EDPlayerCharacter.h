// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AttributeSet.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "UI/HUD/EDFloatingHealthBarSource.h"
#include "Weapon/EDWeapon.h"
#include "EDPlayerCharacter.generated.h"

enum class EPlayerNameType : uint8;
class UPlayerAssetComponent;
class UGameplayEffect;
class UGameplayAbility;
class AEDWeapon;
class AEDPlayerController;
class UEDBaseAttributeSet;
class UEDPlayerAttributeSet;
class UIMCComponent;
class UZoneDetectorComponent;
class UEDInventoryComponent;
class USkillComponent;
class UEDGameDataSubsystem;
class UEDFloatingHealthBarWidgetComponent;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttributeChanged);
/*
 * 플레이어 캐릭터 클래스
 */
UCLASS()
class ETERNALDREAMS_API AEDPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEDPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void OnRep_PlayerState() override;

public:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PostInitializeComponents() override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	//Getter	
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	FORCEINLINE UEDPlayerAttributeSet* GetPlayerAttributeSet() const { return PlayerAttributeSet; }
	
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	FORCEINLINE UEDBaseAttributeSet* GetBaseAttributeSet() const { return BaseAttributeSet; }
	
	UFUNCTION()
	FORCEINLINE USkillComponent* GetSkillComponent() const {return PlayerSkillComponent;}

	FORCEINLINE FEDOnFloatingHealthBarSourceChanged& GetOnFloatingHealthBarSourceChanged() { return OnFloatingHealthBarSourceChanged; }
	
	
	
	//Components
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UIMCComponent> IMCComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UZoneDetectorComponent> ZoneDetector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UEDFloatingHealthBarWidgetComponent> FloatingHealthBarWidgetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<USkillComponent> PlayerSkillComponent;
	
	
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;
	
	//AttributeSet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDPlayerAttributeSet> PlayerAttributeSet;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AttributeSet")
	TObjectPtr<UEDBaseAttributeSet> BaseAttributeSet;
	
	//Abilities for Grant
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	//Test Weapon
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AEDWeapon> WeaponClass;
	UPROPERTY()
	TObjectPtr<AEDWeapon> RWeaponActor;
	UPROPERTY()
	TObjectPtr<AEDWeapon> LWeaponActor;
	UPROPERTY()
	FName RWeaponSocketName=FName("handslot_r");
	UPROPERTY()
	FName LWeaponSocketName=FName("handslot_l");
	
	

	//Get Attribute
public:
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float GetMaxHealth() const;

	// -------------------------------------------------------
	// Death
	// -------------------------------------------------------

	/**
	 * 플레이어 사망 처리 진입점. 서버에서만 호출.
	 * HP 0 또는 SurvivalTime 0 감지 시 AttributeSet에서 호출된다.
	 * GA_Death 몽타주 발동 + GameMode에 사망 전달 (관전/부활/탈락 분기)
	 */
	UFUNCTION(BlueprintCallable, Category = "ED|Death")
	void HandleDeath(AController* Killer);

	UFUNCTION(BlueprintCallable, Category = "ED|Death")
	bool IsDead() const { return bIsDead; }

protected:
	bool bIsStop=false;
	
	/** 중복 HandleDeath 호출 방지용 서버 전용 플래그 */
	bool bIsDead = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "EquipEffect")
	TSubclassOf<UGameplayEffect> EquipEffect;
	
	//Initialize AS
	virtual void InitializeAbilitySystem();

	// Ability Grant
	void GiveDefaultAbilities();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;
	
	
	//Callback
	
	void OnStopTagChanged(const FGameplayTag Tag,int32 newCount);
	void OnWalkSpeedChanged(const struct FOnAttributeChangeData& Data);
	void OnHealthChanged(const struct FOnAttributeChangeData& Data);
	UFUNCTION()
	void OnEquipChanged(FGameplayTag& AttributeDataTag, float Value);
	UFUNCTION(BlueprintCallable,Server, Reliable)
	void OnWeaponChanged();
	UFUNCTION(BlueprintCallable,Server, Reliable)
	void OnPlayerSkinChanged(EPlayerNameType SkinName);
	UFUNCTION(BlueprintCallable,Server, Reliable)
	void OnFirstSkillChanged(const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags);
	UFUNCTION(BlueprintCallable,Server, Reliable)
	void OnSecondSkillChanged(const FGameplayTagContainer& SkillItemTags, const FGameplayTagContainer& SkillCooldownTags);
	

	
	
	
	//Skin
#pragma region Skin
protected:
	UPROPERTY(ReplicatedUsing = OnRep_TargetMeshId)
	FPrimaryAssetId TargetMeshId;
	UPROPERTY(ReplicatedUsing = OnRep_TargetABPId)
	FPrimaryAssetId TargetABPId;
	UPROPERTY(ReplicatedUsing = OnRep_RetargetMeshId)
	FPrimaryAssetId RetargetMeshId;
	UPROPERTY(ReplicatedUsing = OnRep_RetargetABPId)
	FPrimaryAssetId RetargetABPId;
	
public:	
	//Delegates
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChanged OnHealthDecreased;
	

	
	
public:
	
	
	UFUNCTION()
	FORCEINLINE void SetTargetMeshId(const FPrimaryAssetId& InTargetMeshId){if (!HasAuthority()){return;} TargetMeshId=InTargetMeshId; ApplyTargetMesh();}
	UFUNCTION()
	FORCEINLINE void SetTargetABPId(const FPrimaryAssetId& InTargetABPId){if (!HasAuthority()){return;} TargetABPId=InTargetABPId; ApplyTargetABP();}
	UFUNCTION()
	FORCEINLINE void SetRetargetMeshId(const FPrimaryAssetId& InRetargetMeshId){if (!HasAuthority()){return;} RetargetMeshId=InRetargetMeshId; ApplyRetargetMesh();}
	UFUNCTION()
	FORCEINLINE void SetRetargetABPId(const FPrimaryAssetId& InRetargetABPId){if (!HasAuthority()){return;} RetargetABPId=InRetargetABPId; ApplyRetargetABP();}
	
	UFUNCTION()
	void ApplyTargetMesh();
	UFUNCTION()
	void ApplyTargetABP();
	UFUNCTION()
	void ApplyRetargetMesh();
	UFUNCTION()
	void ApplyRetargetABP();
	
	UFUNCTION()
	FORCEINLINE void OnRep_TargetMeshId(){ApplyTargetMesh();}
	UFUNCTION()
	FORCEINLINE void OnRep_TargetABPId(){ApplyTargetABP();}
	UFUNCTION()
	FORCEINLINE void OnRep_RetargetMeshId(){ApplyRetargetMesh();}
	UFUNCTION()
	FORCEINLINE void OnRep_RetargetABPId(){ApplyRetargetABP();}
	
	
#pragma endregion
	//Anim Move
#pragma region Animation Movement
	//Caching
protected:
	UPROPERTY()
	bool bIsAnimMoving=false;
	UPROPERTY()
	float DashSpeed=0.f;
	UPROPERTY()
	bool bIsForward=false;
	UPROPERTY()
	bool bIsZ=false;
	UPROPERTY()
	FVector MoveVector=FVector::ZeroVector;
	UPROPERTY()
	FHitResult Hit;
	
	//Get Animation Movement Input
public:
	UFUNCTION()
	void StartAnimMove(float InDashSpeed, bool InbIsForward, bool InbIsZ );
	UFUNCTION()
	void StopAnimMove();
#pragma endregion
	//Anim Notify Used
#pragma region AnimNotifyUsed
public:
	UPROPERTY()
	FVector PresentAttackSocketLocation=FVector::ZeroVector;
	UPROPERTY()
	FVector CurrentAttackSocketLocation=FVector::ZeroVector;
	
	UPROPERTY()
	FTransform SpawnTransform;
	
	UPROPERTY()
	FVector SocketLocation;
	
	UPROPERTY()
	FVector SocketDirection;
	
	UPROPERTY()
	TArray<AActor*> HittedCharacterArray;
	
	FORCEINLINE UStaticMeshComponent* GetWeaponMeshComp()
	{if (LWeaponActor!=nullptr&&RWeaponActor!=nullptr) return RWeaponActor->GetStaticMesh()!=nullptr ? RWeaponActor->GetStaticMeshComp():LWeaponActor->GetStaticMeshComp(); else return nullptr;};
#pragma endregion

private:
	// 플레이어 데이터 로드 및 초기화
	UFUNCTION()
	void ApplyPlayerDataAsset();
	void BroadcastFloatingHealthBarSource();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetWeaponTags(FGameplayTag BasicAttackTag, FGameplayTag EvadeTag, FGameplayTag EvadeCoolTimeTag);
	TWeakObjectPtr<UEDGameDataSubsystem> CachedDataSubsystem;
	FEDOnFloatingHealthBarSourceChanged OnFloatingHealthBarSourceChanged;
	
	
	//Server Sync
public:
	UPROPERTY(ReplicatedUsing=OnRep_bIsReadySetOverlay)
	bool bIsReadySetOverlay=false;
	
	UFUNCTION()
	void OnRep_bIsReadySetOverlay();
	
	UFUNCTION()
	void SetPlayer();
	UFUNCTION(Server,Reliable)
	void NotifyServerPlayerLoadComplete();
	UFUNCTION()
	void OnSeverLoadedComplete();

	
};
