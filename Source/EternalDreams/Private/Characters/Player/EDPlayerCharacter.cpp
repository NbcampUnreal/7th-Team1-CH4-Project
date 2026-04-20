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
#include "Data/EDPlayerAnimDataAsset.h"
#include "Data/EDPlayerDataAsset.h"
#include "Data/EDWeaponDataAsset.h"
#include "Data/EDPlayerDataAsset.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Data/Types/EDPlayerTypes.h"
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
	
	UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(GetWorld());
	if (!DataSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerChar] BeginPlay - GameDataSubsystem 없음"));
		return;
	}
	// 데이터 서브시스템 캐싱
	CachedDataSubsystem = DataSubsystem;
	if (DataSubsystem->IsDataReady())
	{
		UE_LOG(LogTemp, Log, TEXT("[PlayerChar] 데이터 준비 완료 - 즉시 초기화 실행"));
		ApplyPlayerDataAsset();
	} else
	{
		UE_LOG(LogTemp, Log, TEXT("[PlayerChar] 데이터 준비 미완료 - 콜백 초기화 실행"));
		DataSubsystem->OnAllDataLoaded.AddDynamic(this, &AEDPlayerCharacter::ApplyPlayerDataAsset);
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
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(AbilitySpec);
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
    if (!HasAuthority()) return;
    if (!IsValid(InventoryComponent) || !IsValid(AbilitySystemComponent) || !IsValid(PlayerSkillComponent)) return;

    const FEDGameplayTags& EDGameplayTags = FEDGameplayTags::Get();
	
    const UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(GetWorld());
    if (!DataSubsystem) return;

    FName WeaponCategory = *UEnum::GetDisplayValueAsText(EPlayerDataType::WeaponData).ToString();

    FGameplayTag WeaponTagToSet        = FGameplayTag::EmptyTag;
    FGameplayTag BasicAttackTagToSet   = FGameplayTag::EmptyTag;
    FGameplayTag EvadeTagToSet         = FGameplayTag::EmptyTag;
    FGameplayTag EvadeCoolTimeTagToSet = FGameplayTag::EmptyTag;

    // 로드할 메시 소프트 레퍼런스
    TSoftObjectPtr<UStaticMesh> RWeaponMeshPtr;
    TSoftObjectPtr<UStaticMesh> LWeaponMeshPtr;

    if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Hammer))
    {
        UEDWeaponDataAsset* Hammer = DataSubsystem->GetData<UEDWeaponDataAsset>(
            FPrimaryAssetId(WeaponCategory, *UEnum::GetDisplayValueAsText(EWeaponNameType::Hammer).ToString())
        );
        if (Hammer) RWeaponMeshPtr = Hammer->WeaponStaticMesh;
        // 망치는 한 손 무기 → LWeapon 없음

        WeaponTagToSet        = EDGameplayTags.Item_Weapon_Hammer;
        BasicAttackTagToSet   = EDGameplayTags.Player_BasicAttack_Hammer;
        EvadeTagToSet         = EDGameplayTags.Player_Evade_Hammer;
        EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Hammer;
    }
    else if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Bow))
    {
        UEDWeaponDataAsset* Bow = DataSubsystem->GetData<UEDWeaponDataAsset>(
            FPrimaryAssetId(WeaponCategory, *UEnum::GetDisplayValueAsText(EWeaponNameType::Bow).ToString())
        );
        if (Bow) LWeaponMeshPtr = Bow->WeaponStaticMesh;

        WeaponTagToSet        = EDGameplayTags.Item_Weapon_Bow;
        BasicAttackTagToSet   = EDGameplayTags.Player_BasicAttack_Bow;
        EvadeTagToSet         = EDGameplayTags.Player_Evade_Bow;
        EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Bow;
    }
    else if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Staff))
    {
        UEDWeaponDataAsset* Staff = DataSubsystem->GetData<UEDWeaponDataAsset>(
            FPrimaryAssetId(WeaponCategory, *UEnum::GetDisplayValueAsText(EWeaponNameType::Staff).ToString())
        );
        if (Staff) RWeaponMeshPtr = Staff->WeaponStaticMesh;

        WeaponTagToSet        = EDGameplayTags.Item_Weapon_Staff;
        BasicAttackTagToSet   = EDGameplayTags.Player_BasicAttack_Staff;
        EvadeTagToSet         = EDGameplayTags.Player_Evade_Staff;
        EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Staff;
    }
    else if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Sword))
    {
        UEDWeaponDataAsset* Sword = DataSubsystem->GetData<UEDWeaponDataAsset>(
            FPrimaryAssetId(WeaponCategory, *UEnum::GetDisplayValueAsText(EWeaponNameType::Sword).ToString())
        );
        if (Sword) RWeaponMeshPtr = Sword->WeaponStaticMesh;

        WeaponTagToSet        = EDGameplayTags.Item_Weapon_Sword;
        BasicAttackTagToSet   = EDGameplayTags.Player_BasicAttack_Sword;
        EvadeTagToSet         = EDGameplayTags.Player_Evade_Sword;
        EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Sword;
    }

    // 스킬 태그 적용 (메시 로드와 무관하게 즉시)
    if (BasicAttackTagToSet.IsValid())
    {
        MulticastSetWeaponTags(BasicAttackTagToSet, EvadeTagToSet, EvadeCoolTimeTagToSet);
    }

    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

    // RWeapon 비동기 로드
    if (!RWeaponMeshPtr.IsNull())
    {
        if (RWeaponMeshPtr.IsValid())
        {
            // 이미 메모리에 있음 → 바로 적용
            if (IsValid(RWeaponActor))
                RWeaponActor->ApplyMeshOnServer(RWeaponMeshPtr.Get());
        }
        else
        {
            // 비동기 로드 후 콜백에서 적용
            Streamable.RequestAsyncLoad(
                RWeaponMeshPtr.ToSoftObjectPath(),
                FStreamableDelegate::CreateUObject(this, &AEDPlayerCharacter::OnRWeaponMeshLoaded, RWeaponMeshPtr)
            );
        }
    }
    else
    {
        if (IsValid(RWeaponActor))
            RWeaponActor->SetServerStaticMesh(nullptr);
    }

    // LWeapon 비동기 로드
    if (!LWeaponMeshPtr.IsNull())
    {
        if (LWeaponMeshPtr.IsValid())
        {
            if (IsValid(LWeaponActor))
                LWeaponActor->ApplyMeshOnServer(LWeaponMeshPtr.Get());
        }
        else
        {
            Streamable.RequestAsyncLoad(
                LWeaponMeshPtr.ToSoftObjectPath(),
                FStreamableDelegate::CreateUObject(this, &AEDPlayerCharacter::OnLWeaponMeshLoaded, LWeaponMeshPtr)
            );
        }
    }
    else
    {
        if (IsValid(LWeaponActor))
            LWeaponActor->SetServerStaticMesh(nullptr);
    }
}

void AEDPlayerCharacter::OnPlayerSkinChanged(EPlayerNameType& SkinName)
{
	if (!IsValid(GetWorld()))
	{
		return;
	}
	
	UEDGameDataSubsystem* EDGameDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (!IsValid(EDGameDataSubsystem))
	{
		return;
	}
	
	FName PlayerSkinCategory= *UEnum::GetDisplayValueAsText(EPlayerDataType::PlayerData).ToString();
	
	//타겟 메시 및 애님인스턴스 설정
	if (GetMesh()==nullptr||GetMesh()->GetAnimInstance()==nullptr)
	{
		UEDPlayerDataAsset* TargetSkin = EDGameDataSubsystem->GetData<UEDPlayerDataAsset>(
		FPrimaryAssetId(
			PlayerSkinCategory,
			*UEnum::GetDisplayValueAsText(EPlayerNameType::Basic).ToString()
			));
		if (TargetSkin)
		{
			GetMesh()->SetSkeletalMesh(TargetSkin->SkeletalMesh.Get());
			GetMesh()->SetAnimInstanceClass(TargetSkin->AnimationBlueprint.Get());
		}
	}
	
	//플레이어 스킨 변경
	UEDPlayerDataAsset* PlayerSkin = EDGameDataSubsystem->GetData<UEDPlayerDataAsset>(
		FPrimaryAssetId(
			PlayerSkinCategory,
			*UEnum::GetDisplayValueAsText(SkinName).ToString()
			));
	
	USkeletalMeshComponent* RetargetMesh=Cast<USkeletalMeshComponent>(GetMesh()->GetChildComponent(0));
	
	if (PlayerSkin&&IsValid(RetargetMesh))
	{
		RetargetMesh->SetSkeletalMesh(PlayerSkin->SkeletalMesh.Get());
		RetargetMesh->SetAnimInstanceClass(PlayerSkin->AnimationBlueprint.Get());
	}
}

void AEDPlayerCharacter::OnRWeaponMeshLoaded(TSoftObjectPtr<UStaticMesh> MeshPtr)
{
	if (IsValid(RWeaponActor) && MeshPtr.IsValid())
	{
		RWeaponActor->ApplyMeshOnServer(MeshPtr.Get());
	}
}

void AEDPlayerCharacter::OnLWeaponMeshLoaded(TSoftObjectPtr<UStaticMesh> MeshPtr)
{
	if (IsValid(LWeaponActor) && MeshPtr.IsValid())
	{
		LWeaponActor->ApplyMeshOnServer(MeshPtr.Get());
	}
}

void AEDPlayerCharacter::MulticastSetWeaponTags_Implementation(FGameplayTag BasicAttackTag, FGameplayTag EvadeTag, FGameplayTag EvadeCoolTimeTag)
{
	if (!IsValid(PlayerSkillComponent)) return;

	PlayerSkillComponent->SetBasicAttackTag(BasicAttackTag);
	PlayerSkillComponent->SetSpaceSkillTag(EvadeTag);
	PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EvadeCoolTimeTag);
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
//  데이터 처리
// ============================================================

void AEDPlayerCharacter::ApplyPlayerDataAsset()
{
    if (!HasAuthority()) return;

    UEDGameDataSubsystem* DataSubsystem = CachedDataSubsystem.Get();
    if (!DataSubsystem) return;

    FName PlayerDataCategory = *UEnum::GetDisplayValueAsText(EPlayerDataType::PlayerData).ToString();
    UEDPlayerDataAsset* PlayerData = DataSubsystem->GetData<UEDPlayerDataAsset>(
        FPrimaryAssetId(PlayerDataCategory, *UEnum::GetDisplayValueAsText(EPlayerNameType::Basic).ToString())
    );
    if (!PlayerData) return;

    if (PlayerData->SkeletalMesh.IsValid())
    {
        USkeletalMesh* SkeletalMesh = PlayerData->SkeletalMesh.IsPending()
            ? PlayerData->SkeletalMesh.LoadSynchronous()
            : PlayerData->SkeletalMesh.Get();
        if (SkeletalMesh) GetMesh()->SetSkeletalMesh(SkeletalMesh);
    }

    if (PlayerData->AnimationBlueprint.IsValid())
    {
        UClass* AnimBPClass = PlayerData->AnimationBlueprint.IsPending()
            ? PlayerData->AnimationBlueprint.LoadSynchronous()
            : PlayerData->AnimationBlueprint.Get();
        if (AnimBPClass) GetMesh()->SetAnimInstanceClass(AnimBPClass);
    }

    if (!IsValid(WeaponClass))
    {
        UE_LOG(LogTemp, Error, TEXT("[ApplyPlayerDataAsset] WeaponClass 없음"));
        return;
    }

    if (!IsValid(RWeaponActor))
    {
        RWeaponActor = GetWorld()->SpawnActor<AEDWeapon>(WeaponClass);
        if (IsValid(RWeaponActor))
        {
            RWeaponActor->SetOwner(this);
            GetCapsuleComponent()->IgnoreActorWhenMoving(RWeaponActor, true);
        }
    }

    if (!IsValid(LWeaponActor))
    {
        LWeaponActor = GetWorld()->SpawnActor<AEDWeapon>(WeaponClass);
        if (IsValid(LWeaponActor))
        {
            LWeaponActor->SetOwner(this);
            GetCapsuleComponent()->IgnoreActorWhenMoving(LWeaponActor, true);
        }
    }

    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        USkeletalMeshComponent* AttachTarget = GetMesh();

        if (IsValid(RWeaponActor))
        {
            RWeaponActor->AttachToComponent(
                AttachTarget,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                RWeaponSocketName
            );
        }

        if (IsValid(LWeaponActor))
        {
            LWeaponActor->AttachToComponent(
                AttachTarget,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                LWeaponSocketName
            );
        }
    	if (IsValid(InventoryComponent))
    	{
			InventoryComponent->RequestEnsureDefaultEquipment();
		}
    });
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