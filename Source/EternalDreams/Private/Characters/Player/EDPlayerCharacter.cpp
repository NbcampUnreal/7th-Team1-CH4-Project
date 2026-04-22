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
#include "Core/EDPlayerState.h"
#include "Core/EDSkillDataSubsystem.h"
#include "Data/EDPlayerAnimDataAsset.h"
#include "Data/EDPlayerDataAsset.h"
#include "Data/EDWeaponDataAsset.h"
#include "Data/EDPlayerDataAsset.h"
#include "Data/EDSkillDeveloperSettings.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Data/Types/EDPlayerTypes.h"
#include "Engine/AssetManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Inventory/Component/EDInventoryComponent.h"
#include "Item/Data/EDInventoryItemDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "UI/HUD/EDFloatingHealthBarWidgetComponent.h"


// Sets default values
AEDPlayerCharacter::AEDPlayerCharacter()
{
	bReplicates=true;
	
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

	FloatingHealthBarWidgetComponent = CreateDefaultSubobject<UEDFloatingHealthBarWidgetComponent>(TEXT("FloatingHealthBarWidgetComponent"));
	FloatingHealthBarWidgetComponent->SetupAttachment(GetRootComponent());
	FloatingHealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	FloatingHealthBarWidgetComponent->SetDrawAtDesiredSize(true);
	FloatingHealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
	FloatingHealthBarWidgetComponent->SetVisibility(true);

	InventoryComponent = CreateDefaultSubobject<UEDInventoryComponent>(TEXT("InventoryComponent"));
}

// Called when the game starts or when spawned
void AEDPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//서버에서 AbilitySystem 초기화
	if (HasAuthority())
	{
		InitializeAbilitySystem();
	}
	
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
		ApplyPlayerDataAsset();
	} else
	{
		DataSubsystem->OnAllDataLoaded.AddDynamic(this, &AEDPlayerCharacter::ApplyPlayerDataAsset);
	}
	//Inventory Binding
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnInventoryChanged.AddDynamic(this,&AEDPlayerCharacter::OnWeaponChanged);
		InventoryComponent->OnFirstSkillSlotChanged.AddDynamic(this,&AEDPlayerCharacter::OnFirstSkillChanged);
		InventoryComponent->OnSecondSkillSlotChanged.AddDynamic(this,&AEDPlayerCharacter::OnSecondSkillChanged);
	}
	
	
	
	//ASC Duration Callback
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BaseAttributeSet->GetWalkSpeedAttribute())
	.AddUObject(this, &AEDPlayerCharacter::OnWalkSpeedChanged);

	BroadcastFloatingHealthBarSource();
	if (IsLocallyControlled())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(BaseAttributeSet->GetHealthAttribute())
    .AddUObject(this, &AEDPlayerCharacter::OnHealthChanged);
	}

	AbilitySystemComponent->RegisterGameplayTagEvent(FEDGameplayTags::Get().State_Player_Stop,EGameplayTagEventType::NewOrRemoved).
	AddUObject(this,&AEDPlayerCharacter::OnStopTagChanged);
	
	
}

void AEDPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	FHitResult HitResult;
	if (UGameplayStatics::GetPlayerController(GetWorld(),0)->GetHitResultUnderCursor(ECC_Visibility,false, HitResult))
	{
		if (!bIsStop&&IsLocallyControlled())
		{
			FVector TargetLocation = HitResult.ImpactPoint;
			FVector StartLocation = GetActorLocation();
		
			// 방향 Rotator 계산(Yaw만 사용)
			FRotator LookAtRotation = FRotationMatrix::MakeFromX(TargetLocation - StartLocation).Rotator();
			LookAtRotation.Pitch = 0.0f;
			LookAtRotation.Roll = 0.0f;
		
			UGameplayStatics::GetPlayerController(GetWorld(),0)->SetControlRotation(LookAtRotation);
		}
	}
	
	
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



void AEDPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	//AbilitySystem 초기화
	InitializeAbilitySystem();
	BroadcastFloatingHealthBarSource();
	UE_LOG(LogTemp,Warning,TEXT("OnRep_PlayerState"));
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC && LocalPC->PlayerState && GetPlayerState())
	{
		
		UE_LOG(LogTemp,Warning,TEXT("OnRep_PlayerState : Character"));
		if (!IsLocallyControlled())
		{
			UE_LOG(LogTemp,Warning,TEXT("OnRep_PlayerState : OtherCharacter"));
			AEDPlayerState* ThisPS=Cast<AEDPlayerState> (GetPlayerState());
			AEDPlayerState* LocalPS=Cast<AEDPlayerState> (LocalPC->PlayerState);	
			
			if (ThisPS&&LocalPS)
			{
				UE_LOG(LogTemp,Warning,TEXT("%d %d"),ThisPS->TeamId,LocalPS->TeamId);
				if (GetMesh()&&GetMesh()->GetChildComponent(0))
				{
					USkeletalMeshComponent* RetargetMeshComp=Cast<USkeletalMeshComponent>(GetMesh()->GetChildComponent(0));
					UEDSkillDataSubsystem* SkillDataSubsystem = UEDSkillDataSubsystem::Get(GetWorld());
					if (RetargetMeshComp&&SkillDataSubsystem)
					{
						//적군 오버레이 머티리얼 설정
						if (ThisPS->TeamId!=LocalPS->TeamId)
						{
							RetargetMeshComp->SetOverlayMaterial(SkillDataSubsystem->GetEnemyMat());
						}
						//팀 오버레이 머티리얼 설정
						else
						{
							RetargetMeshComp->SetOverlayMaterial(SkillDataSubsystem->GetTeamMat());
						}
					}
				}
			}
			
		}
	}
	
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

void AEDPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AEDPlayerCharacter,TargetMeshId );
	DOREPLIFETIME(AEDPlayerCharacter,TargetABPId );
	DOREPLIFETIME(AEDPlayerCharacter,RetargetMeshId );
	DOREPLIFETIME(AEDPlayerCharacter,RetargetABPId );

}

void AEDPlayerCharacter::ApplyTargetMesh()
{
	if (!TargetMeshId.IsValid())
	{
		return;
	}
	
	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (!EDGameplayDataSubsystem)
	{
		return;
	}
	
	UEDPlayerDataAsset* TargetMesh = 
			EDGameplayDataSubsystem->GetData<UEDPlayerDataAsset>(TargetMeshId
				);
		
	if (IsValid(TargetMesh))
	{
		GetMesh()->SetSkeletalMesh(TargetMesh->SkeletalMesh.LoadSynchronous());
	}
}

void AEDPlayerCharacter::ApplyTargetABP()
{
	if (!TargetABPId.IsValid())
	{
		return;
	}
	
	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (!EDGameplayDataSubsystem)
	{
		return;
	}
	
	UEDPlayerDataAsset* TargetABP = 
			EDGameplayDataSubsystem->GetData<UEDPlayerDataAsset>(TargetABPId
				);
		
	if (IsValid(TargetABP))
	{
		GetMesh()->SetAnimInstanceClass(TargetABP->AnimationBlueprint.LoadSynchronous());
	}
}

void AEDPlayerCharacter::ApplyRetargetMesh()
{
	if (!RetargetMeshId.IsValid())
	{
		return;
	}
	
	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (!EDGameplayDataSubsystem)
	{
		return;
	}
	
	UEDPlayerDataAsset* RetargetMesh = 
			EDGameplayDataSubsystem->GetData<UEDPlayerDataAsset>(RetargetMeshId
				);
	
	USkeletalMeshComponent* RetargetMeshComp=Cast<USkeletalMeshComponent>(GetMesh()->GetChildComponent(0));
		
	if (IsValid(RetargetMesh)&&IsValid(RetargetMeshComp))
	{
		RetargetMeshComp->SetSkeletalMesh(RetargetMesh->SkeletalMesh.LoadSynchronous());
	}
}

void AEDPlayerCharacter::ApplyRetargetABP()
{
	if (!RetargetABPId.IsValid())
	{
		return;
	}
	
	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (!EDGameplayDataSubsystem)
	{
		return;
	}
	
	UEDPlayerDataAsset* RetargetABP = 
			EDGameplayDataSubsystem->GetData<UEDPlayerDataAsset>(RetargetABPId
				);
	USkeletalMeshComponent* RetargetMeshComp=Cast<USkeletalMeshComponent>(GetMesh()->GetChildComponent(0));
	if (IsValid(RetargetABP))
	{
		RetargetMeshComp->SetAnimInstanceClass(RetargetABP->AnimationBlueprint.LoadSynchronous());
	}
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

void AEDPlayerCharacter::OnStopTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount>0)
	{
		bIsStop=true;
	}
	else
	{
		bIsStop=false;
	}
}

void AEDPlayerCharacter::OnWalkSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed=Data.NewValue;
}

void AEDPlayerCharacter::OnHealthChanged(const struct FOnAttributeChangeData& Data)
{
	//Health가 이전 값이 변경된 값보다 크다면 (=체력이 감소했다면)
	if (Data.OldValue>Data.NewValue)
	{
		OnHealthDecreased.Broadcast();
	}
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
void AEDPlayerCharacter::OnFirstSkillChanged_Implementation(const FGameplayTagContainer& SkillItemTags,
															const FGameplayTagContainer& SkillCooldownTags)
{
	if (SkillItemTags.IsEmpty()||SkillCooldownTags.IsEmpty())
	{
		return;
	}
	PlayerSkillComponent->SetQSkillTag(SkillItemTags.GetByIndex(0));
	PlayerSkillComponent->SetQSkillCoolTimeTag(SkillCooldownTags.GetByIndex(0));
}

void AEDPlayerCharacter::OnSecondSkillChanged_Implementation(const FGameplayTagContainer& SkillItemTags,
	const FGameplayTagContainer& SkillCooldownTags)
{
	if (SkillItemTags.IsEmpty()||SkillCooldownTags.IsEmpty())
	{
		return;
	}
	PlayerSkillComponent->SetESkillTag(SkillItemTags.GetByIndex(0));
	PlayerSkillComponent->SetESkillCoolTimeTag(SkillCooldownTags.GetByIndex(0));
}


void AEDPlayerCharacter::OnWeaponChanged_Implementation()
{
	if (!IsValid(InventoryComponent)||
		!IsValid(AbilitySystemComponent)||
		!IsValid(PlayerSkillComponent))
	{
		return;
	}
	
	if (!IsValid(LWeaponActor) || !IsValid(RWeaponActor))
	{
		// 인벤토리 변경이 무기 액터 생성보다 먼저 들어올 수 있으므로
		// 현재 플레이어 데이터 적용 경로를 한 번 더 태워 무기 액터 생성을 재시도
		ApplyPlayerDataAsset();

		// 재시도 이후에도 무기 액터가 없으면 더 진행하지 않음
		if (!IsValid(LWeaponActor) || !IsValid(RWeaponActor))
		{
			return;
		}
	}
	
	const UEDGameDataSubsystem* DataSubsystem = UEDGameDataSubsystem::Get(GetWorld());
	if (!DataSubsystem) return;

	FName WeaponCategory=  *UEnum::GetDisplayValueAsText(EPlayerDataType::WeaponData).ToString();
	
	FEDGameplayTags EDGameplayTags=FEDGameplayTags::Get();
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Bow))
	{
		LWeaponActor->SetStaticMeshId(FPrimaryAssetId(
				WeaponCategory, 
				*UEnum::GetDisplayValueAsText(EWeaponNameType::Bow).ToString()));
		RWeaponActor->SetStaticMeshId(FPrimaryAssetId());
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Bow);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Bow);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Bow);
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Hammer))
	{
		LWeaponActor->SetStaticMeshId(FPrimaryAssetId());
		RWeaponActor->SetStaticMeshId(FPrimaryAssetId(
		WeaponCategory, 
		*UEnum::GetDisplayValueAsText(EWeaponNameType::Hammer).ToString()));
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Hammer);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Hammer);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Hammer);
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Staff))
	{
		LWeaponActor->SetStaticMeshId(FPrimaryAssetId());
		RWeaponActor->SetStaticMeshId(FPrimaryAssetId(
		WeaponCategory, 
		*UEnum::GetDisplayValueAsText(EWeaponNameType::Staff).ToString()));
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Staff);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Staff);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Staff);
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(EDGameplayTags.Item_Weapon_Sword))
	{
		LWeaponActor->SetStaticMeshId(FPrimaryAssetId());
		RWeaponActor->SetStaticMeshId(FPrimaryAssetId(
		WeaponCategory, 
		*UEnum::GetDisplayValueAsText(EWeaponNameType::Sword).ToString()));
		
		PlayerSkillComponent->SetBasicAttackTag(EDGameplayTags.Player_BasicAttack_Sword);
		PlayerSkillComponent->SetSpaceSkillTag(EDGameplayTags.Player_Evade_Sword);
		PlayerSkillComponent->SetSpaceSkillCoolTimeTag(EDGameplayTags.CoolDown_Evade_Sword);	
	}
}

void AEDPlayerCharacter::OnPlayerSkinChanged_Implementation(EPlayerNameType SkinName)
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

		SetTargetMeshId(FPrimaryAssetId(
			PlayerSkinCategory,
			*UEnum::GetDisplayValueAsText(EPlayerNameType::Basic).ToString()
			));
		
		SetTargetABPId(FPrimaryAssetId(PlayerSkinCategory,
			*UEnum::GetDisplayValueAsText(EPlayerNameType::Basic).ToString()
			));
		
	}
	
	//플레이어 스킨 변경
	UEDPlayerDataAsset* PlayerSkin = EDGameDataSubsystem->GetData<UEDPlayerDataAsset>(
		FPrimaryAssetId(
			PlayerSkinCategory,
			*UEnum::GetDisplayValueAsText(SkinName).ToString()
			));
	
	SetRetargetMeshId(FPrimaryAssetId(
		PlayerSkinCategory,
		*UEnum::GetDisplayValueAsText(SkinName).ToString()
		));
	
	SetRetargetABPId(FPrimaryAssetId(
		PlayerSkinCategory,
		*UEnum::GetDisplayValueAsText(SkinName).ToString()
		));
}


void AEDPlayerCharacter::MulticastSetWeaponTags_Implementation(FGameplayTag BasicAttackTag, FGameplayTag EvadeTag, FGameplayTag EvadeCoolTimeTag)
{
	if (!IsValid(PlayerSkillComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("[MulticastSetWeaponTags] PlayerSkillComponent NULL!"));
		return;
	}

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
    	// 만약 메시가 아직 준비안되었으면 동기로 로드, 정상적인 시퀀스에서는 동기로 호출될일 없음
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

void AEDPlayerCharacter::BroadcastFloatingHealthBarSource()
{
	OnFloatingHealthBarSourceChanged.Broadcast(AbilitySystemComponent, BaseAttributeSet);
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
