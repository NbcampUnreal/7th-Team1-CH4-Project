// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/EDPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "Characters/Player/Component/IMCComponent.h"
#include "Characters/Player/Component/ZoneDetectorComponent.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Characters/Player/EDPlayerController.h"
#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Core/EDGameMode.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Characters/Player/Component/SkillComponent.h"
#include "Characters/Player/Weapon/EDWeapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/EDAssetManager.h"
#include "Core/EDGameDataSubsystem.h"
#include "Data/EDWeaponDataAsset.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"


// Sets default values
AEDPlayerCharacter::AEDPlayerCharacter()
{
	// ASC 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	//AttributeSet 생성
	BaseAttributeSet = CreateDefaultSubobject<UEDBaseAttributeSet>(TEXT("BaseAttributeSet"));
	PlayerAttributeSet = CreateDefaultSubobject<UEDPlayerAttributeSet>(TEXT("EDAttributeSet"));
	
	//Skill 컴포넌트 생성
	PlayerSkillComponent=CreateDefaultSubobject<USkillComponent>(TEXT("PlayerSkillComponent"));
	
	//IMC 컴포넌트 생성
	IMCComponent=CreateDefaultSubobject<UIMCComponent>(TEXT("IMCComponent"));
	
	//--ksh 금지구역 감지 컴포넌트 부착
	ZoneDetector = CreateDefaultSubobject<UZoneDetectorComponent>(TEXT("ZoneDetector"));

	InventoryComponent = CreateDefaultSubobject<UEDInventoryComponent>(TEXT("InventoryComponent"));
}

// Called when the game starts or when spawned
void AEDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//AbilitySystem 초기화
	InitializeAbilitySystem();
	
	//서버에서만, ASC가 있는 경우 실행
	if (HasAuthority()&&IsValid(AbilitySystemComponent))
	{
		GiveDefaultAbilities();
	}
	//IMC 추가
	AEDPlayerController* PC = Cast<AEDPlayerController>(GetController());
	if (IsValid(PC))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PC->PlayerInputMappingContext, 0);  // Gameplay
		}
	}
	if (!GetWorld())
	{
		return;
	}
	
	//Weapon Test
	if (HasAuthority())
	{
		RWeaponActor=GetWorld()->SpawnActor<AEDWeapon>(WeaponClass);
		if (IsValid(RWeaponActor))
		{
			SkeletalMeshComp=Cast<USkeletalMeshComponent>(GetMesh()->GetChildComponent(0));
			if (!IsValid(SkeletalMeshComp))
			{
				return;
			}
			RWeaponActor->SetOwner(this);
			RWeaponActor->AttachToComponent(GetMesh()->GetChildComponent(0),FAttachmentTransformRules::SnapToTargetNotIncludingScale,RWeaponSocketName);
			GetCapsuleComponent()->IgnoreActorWhenMoving(RWeaponActor,true);
		}
		LWeaponActor=GetWorld()->SpawnActor<AEDWeapon>(WeaponClass);
		if (IsValid(LWeaponActor))
		{
			SkeletalMeshComp=Cast<USkeletalMeshComponent>(GetMesh()->GetChildComponent(0));
			if (!IsValid(SkeletalMeshComp))
			{
				return;
			}
			LWeaponActor->SetOwner(this);
			LWeaponActor->AttachToComponent(GetMesh()->GetChildComponent(0),FAttachmentTransformRules::SnapToTargetNotIncludingScale,LWeaponSocketName);
			GetCapsuleComponent()->IgnoreActorWhenMoving(LWeaponActor,true);
		}
	}
	
	//ASC Duration Callback
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BaseAttributeSet->GetWalkSpeedAttribute())
	.AddUObject(this, &AEDPlayerCharacter::OnWalkSpeedChanged);
	
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this,&AEDPlayerCharacter::OnWeaponChanged);
	}
	
	
}

void AEDPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!bIsAnimMoving)
	{
		return;
	}
	
	if (bIsForward)
	{
		MoveVector=GetActorForwardVector();
	}
	else if (bIsZ)
	{
		MoveVector=FVector(0,0,1.f);
	}
	else
	{
		MoveVector=FVector::ZeroVector;
	}
	MoveVector*=DashSpeed*DeltaSeconds;
	AddActorWorldOffset(MoveVector, true, &Hit,ETeleportType::None);
}


// Called to bind functionality to input
void AEDPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
 {
 	Super::SetupPlayerInputComponent(PlayerInputComponent);
 	if (IsValid(IMCComponent))
 	{
 		IMCComponent->SetupPlayerInput(PlayerInputComponent);
 	}
 }

void AEDPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	if (!IsValid(IMCComponent)||!IsValid(PlayerSkillComponent))
	{
		return;
	}
	IMCComponent->OnBasicAttackInput.BindUObject(PlayerSkillComponent,&USkillComponent::ActivateBasicAttack);
	IMCComponent->OnQSkillInput.BindUObject(PlayerSkillComponent,&USkillComponent::ActivateQSkill);
	IMCComponent->OnESkillInput.BindUObject(PlayerSkillComponent,&USkillComponent::ActivateESkill);
	IMCComponent->OnSpaceSkillInput.BindUObject(PlayerSkillComponent,&USkillComponent::ActivateSpaceSkill);
}


UAbilitySystemComponent* AEDPlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AEDPlayerCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AEDPlayerCharacter::GiveDefaultAbilities()
{
	for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			// Ability Spec 생성
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			// ASC에 Ability 부여
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

void AEDPlayerCharacter::StartAnimMove(float InDashSpeed, bool InbIsForward, bool InbIsZ)
{
	DashSpeed=InDashSpeed;
	bIsForward=InbIsForward;
	bIsZ=InbIsZ;
	bIsAnimMoving=true;
}

void AEDPlayerCharacter::StopAnimMove()
{
	bIsAnimMoving=false;
}

void AEDPlayerCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed=Data.NewValue;
}

void AEDPlayerCharacter::OnEquipChanged(FGameplayTag& AttributeDataTag, float Value)
{
	if (!EquipEffect||!AbilitySystemComponent)
	{
		return;
	}
	
	//EffectContext 생성
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);

	//EffectSpec 생성
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
		EquipEffect, 1.0f, Context);

	if (Spec.IsValid())
	{
		Spec.Data.Get()->SetSetByCallerMagnitude(AttributeDataTag, Value);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
	
}

void AEDPlayerCharacter::OnWeaponChanged()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (!IsValid(InventoryComponent)||
		!IsValid(AbilitySystemComponent)||
		!IsValid(PlayerSkillComponent))
	{
		return;
	}
	
	const FEDGameplayTags& EDGameplayTags=FEDGameplayTags::Get();
	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (!EDGameplayDataSubsystem)
	{
		return;
	}
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Bow))
	{
		UEDWeaponDataAsset* Bow = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(TEXT("WeaponData"), TEXT("DA_Bow")));
		
		LWeaponActor->SetServerStaticMesh(Bow->WeaponStaticMesh.Get());
		RWeaponActor->SetServerStaticMesh(nullptr);
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Bow);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Bow);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Bow);
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Hammer))
	{
		UEDWeaponDataAsset* Hammer = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(TEXT("WeaponData"), TEXT("DA_Hammer")));
		
		LWeaponActor->SetServerStaticMesh(nullptr);
		RWeaponActor->SetServerStaticMesh(Hammer->WeaponStaticMesh.Get());
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Hammer);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Hammer);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Hammer);
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Staff))
	{
		UEDWeaponDataAsset* Staff = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(TEXT("WeaponData"), TEXT("DA_Staff")));
		
		LWeaponActor->SetServerStaticMesh(nullptr);
		RWeaponActor->SetServerStaticMesh(Staff->WeaponStaticMesh.Get());
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Staff);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Staff);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Staff);
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Sword))
	{
		UEDWeaponDataAsset* Sword = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(TEXT("WeaponData"), TEXT("DA_Sword")));
		
		LWeaponActor->SetServerStaticMesh(nullptr);
		RWeaponActor->SetServerStaticMesh(Sword->WeaponStaticMesh.Get());
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Sword);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Sword);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Sword);	
	}
	

}

float AEDPlayerCharacter::GetHealth() const
{
	if (BaseAttributeSet)
	{
		return BaseAttributeSet->GetHealth();
	}
	return 0.0f;
}

float AEDPlayerCharacter::GetMaxHealth() const
{
	if (BaseAttributeSet)
	{
		return BaseAttributeSet->GetMaxHealth();
	}
	return 0.0f;
}

// ============================================================
//  사망 처리
// ============================================================

void AEDPlayerCharacter::HandleDeath(AController* Killer)
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}
	bIsDead = true;

	// 이동/충돌 차단 (관전 전환 전에 플레이어가 움직이지 못하게)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->StopMovementImmediately();
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// GA_Death 어빌리티 발동 (몽타주 재생)
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer DeathTag;
		DeathTag.AddTag(FEDGameplayTags::Get().State_Dead);
		AbilitySystemComponent->TryActivateAbilitiesByTag(DeathTag);
	}

	// GameMode에 사망 전달 → 관전/부활/탈락 분기
	if (AEDGameMode* GM = Cast<AEDGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->HandlePlayerDeath(GetController(), Killer);
	}
}

