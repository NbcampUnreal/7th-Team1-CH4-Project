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
	UE_LOG(LogTemp, Warning, TEXT("[GiveDefaultAbilities] 기본 어빌리티 부여 시작 - 개수: %d"), DefaultAbilities.Num());
	
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
	
	UE_LOG(LogTemp, Warning, TEXT("[GiveDefaultAbilities] 기본 어빌리티 부여 완료"));
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
		return;  // 무기 메시 설정은 서버만 처리
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

	FName WeaponCategory=  *UEnum::GetDisplayValueAsText(EPlayerDataType::WeaponData).ToString();
	
	// 무기별 태그 결정
	FGameplayTag WeaponTagToSet = FGameplayTag::EmptyTag;
	FGameplayTag BasicAttackTagToSet = FGameplayTag::EmptyTag;
	FGameplayTag EvadeTagToSet = FGameplayTag::EmptyTag;
	FGameplayTag EvadeCoolTimeTagToSet = FGameplayTag::EmptyTag;
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Bow))
	{
		UEDWeaponDataAsset* Bow = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(
				WeaponCategory, 
				*UEnum::GetDisplayValueAsText(EWeaponNameType::Bow).ToString()
				));
		
		LWeaponActor->SetServerStaticMesh(Bow->WeaponStaticMesh.Get());
		RWeaponActor->SetServerStaticMesh(nullptr);
		
		WeaponTagToSet = EDGameplayTags.Item_Weapon_Bow;
		BasicAttackTagToSet = EDGameplayTags.Player_BasicAttack_Bow;
		EvadeTagToSet = EDGameplayTags.Player_Evade_Bow;
		EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Bow;
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Hammer))
	{
		UEDWeaponDataAsset* Hammer = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(
				WeaponCategory, 
				*UEnum::GetDisplayValueAsText(EWeaponNameType::Hammer).ToString()
				));
		
		LWeaponActor->SetServerStaticMesh(nullptr);
		RWeaponActor->SetServerStaticMesh(Hammer->WeaponStaticMesh.Get());
		
		WeaponTagToSet = EDGameplayTags.Item_Weapon_Hammer;
		BasicAttackTagToSet = EDGameplayTags.Player_BasicAttack_Hammer;
		EvadeTagToSet = EDGameplayTags.Player_Evade_Hammer;
		EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Hammer;
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Staff))
	{
		UEDWeaponDataAsset* Staff = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(
				WeaponCategory, 
				*UEnum::GetDisplayValueAsText(EWeaponNameType::Staff).ToString()
				));
		
		LWeaponActor->SetServerStaticMesh(nullptr);
		RWeaponActor->SetServerStaticMesh(Staff->WeaponStaticMesh.Get());
		
		WeaponTagToSet = EDGameplayTags.Item_Weapon_Staff;
		BasicAttackTagToSet = EDGameplayTags.Player_BasicAttack_Staff;
		EvadeTagToSet = EDGameplayTags.Player_Evade_Staff;
		EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Staff;
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Sword))
	{
		UEDWeaponDataAsset* Sword = 
			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(FPrimaryAssetId(
				WeaponCategory, 
				*UEnum::GetDisplayValueAsText(EWeaponNameType::Sword).ToString()
				));
		
		LWeaponActor->SetServerStaticMesh(nullptr);
		RWeaponActor->SetServerStaticMesh(Sword->WeaponStaticMesh.Get());
		
		WeaponTagToSet = EDGameplayTags.Item_Weapon_Sword;
		BasicAttackTagToSet = EDGameplayTags.Player_BasicAttack_Sword;
		EvadeTagToSet = EDGameplayTags.Player_Evade_Sword;
		EvadeCoolTimeTagToSet = EDGameplayTags.CoolDown_Evade_Sword;
	}
	
	// 모든 클라이언트에서 태그 설정 (멀티캐스트)
	if (BasicAttackTagToSet.IsValid())
	{
		MulticastSetWeaponTags(BasicAttackTagToSet, EvadeTagToSet, EvadeCoolTimeTagToSet);
	}
}

void AEDPlayerCharacter::MulticastSetWeaponTags_Implementation(FGameplayTag BasicAttackTag, FGameplayTag EvadeTag, FGameplayTag EvadeCoolTimeTag)
{
	if (!IsValid(PlayerSkillComponent))
	{
		return;
	}
	
	// 모든 클라이언트에서 실행됨
	PlayerSkillComponent->SetBasicAttackTag(BasicAttackTag);
	PlayerSkillComponent->SetSpaceSkillTag(EvadeTag);
	PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EvadeCoolTimeTag);
	
	UE_LOG(LogTemp, Warning, TEXT("[MulticastSetWeaponTags] 무기 태그 설정 - BasicAttack: %s"), *BasicAttackTag.ToString());
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
	// 서버에서만 실행
	if (!HasAuthority()) return;
	
	// 중복 호출 방지
	// if (bPlayerDataAssetApplied)
	// {
	// 	return;
	// }
	// bPlayerDataAssetApplied = true;
	
	UEDGameDataSubsystem* DataSubsystem = CachedDataSubsystem.Get();
	if (!DataSubsystem) return;
	
	FName PlayerDataCategory = *UEnum::GetDisplayValueAsText(EPlayerDataType::PlayerData).ToString();
	UEDPlayerDataAsset* PlayerData = DataSubsystem->GetData<UEDPlayerDataAsset>(
		FPrimaryAssetId(PlayerDataCategory, *UEnum::GetDisplayValueAsText(EPlayerNameType::Basic).ToString())
	);
	
	if (!PlayerData) return;
	
	if (PlayerData->SkeletalMesh.IsValid())
	{
		USkeletalMesh* SkeletalMesh = PlayerData->SkeletalMesh.Get();
		if (!SkeletalMesh)
		{
			// 보험용 동기로드
			SkeletalMesh = PlayerData->SkeletalMesh.LoadSynchronous();
		}
		
		if (SkeletalMesh)
		{
			GetMesh()->SetSkeletalMesh(SkeletalMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AEDPlayerCharacter] 스켈레탈 메시 로드 실패"));
		}
	}
	
	if (PlayerData->AnimationBlueprint.IsValid())
	{
		// 이미 로드됨
		UClass* AnimBPClass = PlayerData->AnimationBlueprint.Get();
		if (!AnimBPClass)
		{
			// 보험
			AnimBPClass = PlayerData->AnimationBlueprint.LoadSynchronous();
		}
		
		if (AnimBPClass)
		{
			GetMesh()->SetAnimClass(AnimBPClass);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AEDPlayerCharacter] 애니메이션 블루프린트 로드 실패"));
		}
	}
	
	FName WeaponCategory = *UEnum::GetDisplayValueAsText(EPlayerDataType::WeaponData).ToString();
	UEDWeaponDataAsset* HammerData = DataSubsystem->GetData<UEDWeaponDataAsset>(
		FPrimaryAssetId(WeaponCategory, *UEnum::GetDisplayValueAsText(EWeaponNameType::Hammer).ToString())
	);
	
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
            // 망치 스태틱 메시 적용
            if (HammerData)
            {
            	UStaticMesh* WeaponMesh = nullptr;
            	WeaponMesh = HammerData->WeaponStaticMesh.IsPending()
			? HammerData->WeaponStaticMesh.LoadSynchronous()
			: HammerData->WeaponStaticMesh.Get();
                // RWeaponActor->SetServerStaticMesh(HammerData->WeaponStaticMesh.Get());
            	RWeaponActor->SetServerStaticMesh(WeaponMesh);
            }
        }
    }

    if (!IsValid(LWeaponActor))
    {
        LWeaponActor = GetWorld()->SpawnActor<AEDWeapon>(WeaponClass);
        if (IsValid(LWeaponActor))
        {
            LWeaponActor->SetOwner(this);
            GetCapsuleComponent()->IgnoreActorWhenMoving(LWeaponActor, true);
            LWeaponActor->SetServerStaticMesh(nullptr);
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

        if (IsValid(AbilitySystemComponent))
        {
            const FEDGameplayTags& EDGameplayTags = FEDGameplayTags::Get();
            // 기본 무기는 Hammer
            AbilitySystemComponent->AddLooseGameplayTag(EDGameplayTags.Item_Weapon_Hammer);
        	
            OnWeaponChanged();
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

