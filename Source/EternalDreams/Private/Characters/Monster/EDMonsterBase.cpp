// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/EDMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Characters/Monster/EDMonsterAnimInstance.h"
#include "Data/EDMonsterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/AssetManager.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Core/EDGameDataSubsystem.h"
#include "Core/EDAssetManager.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Inventory/Component/EDInventoryComponent.h"

// Sets default values
AEDMonsterBase::AEDMonsterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	// 기본적으로 내부에서 true지만 명시적으로 표시
	SetReplicateMovement(true);
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// 몬스터 GE는 서버만 가지고있고 계산하기 때문에 Minimal로 설정
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	// Idle상태로 시작
	MonsterState = EMonsterState::Idle;
	
	BaseAttributeSet = CreateDefaultSubobject<UEDBaseAttributeSet>(TEXT("BaseAttributeSet"));
	
	// 몬스터는 장비/기본무기 X
	InventoryComponent = CreateDefaultSubobject<UEDInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->bGiveDefaultWeaponOnBeginPlay = false;
	InventoryComponent->bUseEquipmentSlots = false;
	InventoryComponent->bAutoInitializeLootOnBeginPlay = false;
}

// Called when the game starts or when spawned
void AEDMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(AbilitySystemComponent) == false)
		return;
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	OriginLocation = GetActorLocation();
	
	CachedDataSubsystem = UEDGameDataSubsystem::Get(this);
	
	UEDMonsterDataAsset* DataAsset = GetDataAsset();
	if (IsValid(DataAsset) == false)
		return;
	LoadVisuals(DataAsset);
	
	//InitializeFromDataAsset(DataAsset);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UEDBaseAttributeSet::GetHealthAttribute())
	.AddUObject(this, &AEDMonsterBase::OnHealthChanged);
	
	if (HasAuthority() == false)
		return;
	
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DataAsset->GetDefaultAbilities())
	{
		if (IsValid(AbilityClass) == false)
			continue;
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass));
	}
}

void AEDMonsterBase::InitializeFromDataAsset(UEDMonsterDataAsset* InDataAsset)
{
	if (InDataAsset == nullptr)
		return;
	
	MonsterDataId = InDataAsset->GetPrimaryAssetId();
	
	UEDGameDataSubsystem* DataSubsystem = CachedDataSubsystem.Get();
	if (!DataSubsystem) return;
	
	if (DataSubsystem->IsDataReady())
	{
		if (USkeletalMesh* SkelMesh = InDataAsset->GetMesh().Get())
		{
			GetMesh()->SetSkeletalMeshAsset(SkelMesh);
		}
        
		if (UClass* AnimClass = InDataAsset->GetAnimInstance().Get())
		{
			GetMesh()->SetAnimInstanceClass(AnimClass);
		}
	}
	else
	{
		// Mesh/AnimInstance 비동기 로드(임시) - 서버/클라이언트 공통이라 HasAuthority체크 이전 
		LoadVisuals(InDataAsset);
	}
	
	if (HasAuthority() == false)
		return;
	// AttributeSet에 DA의 Stat 적용
	const FMonsterStatRow& Stat = InDataAsset->GetStat();
	UE_LOG(LogTemp, Warning, TEXT("[%s] InitializeFromDataAsset - MaxHP: %.1f, Atk: %.1f"),
		*GetName(), Stat.MaxHP, Stat.Atk)
	if (IsValid(BaseAttributeSet) == false)
		return;
	BaseAttributeSet->InitMaxHealth(Stat.MaxHP);
	BaseAttributeSet->InitHealth(Stat.MaxHP);
	BaseAttributeSet->InitMaxDefensive(Stat.Def);
	BaseAttributeSet->InitMaxWalkSpeed(Stat.MoveSpeed);
	BaseAttributeSet->InitWalkSpeed(Stat.MoveSpeed);
	// 몬스터 이동속도 MovementComponent에 적용
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
		return;
	MoveComp->MaxWalkSpeed = Stat.MoveSpeed;
	MoveComp->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
	// InventoryComponent LootTable 세팅
	InventoryComponent->RandomLootTable = InDataAsset->GetLootTable();
	InventoryComponent->RandomLootRollCount = InDataAsset->GetLootRollCount();
	// DA 초기화 완료 알림
	OnDataAssetInitialized.Broadcast();
}

void AEDMonsterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEDMonsterBase, MonsterState);
	DOREPLIFETIME(AEDMonsterBase, MonsterDataId);
}

void AEDMonsterBase::OnRep_MonsterState()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] Monster State: %d"), *GetName(), (int32)MonsterState);
	
	if (MonsterState == EMonsterState::Dead)
	{
		// 캡슐 콜리전 비활성화
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UEDMonsterAnimInstance* Anim = Cast<UEDMonsterAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(Anim) == false)
		return;
	
	Anim->SetMonsterState(MonsterState);
}

void AEDMonsterBase::OnRep_MonsterDataId()
{
	UEDMonsterDataAsset* DataAsset = GetDataAsset();
	if (IsValid(DataAsset) == false)
		return;
	LoadVisuals(DataAsset);
}

void AEDMonsterBase::LoadVisuals(UEDMonsterDataAsset* InDataAsset)
{
	if (!InDataAsset) return;
	
	UEDGameDataSubsystem* DataSubsystem = CachedDataSubsystem.Get();
	if (DataSubsystem && DataSubsystem->IsDataReady())
	{
		OnVisualsLoaded();
		return;
	}
	
	// 이전 로드가 진행 중이면 취소
	if (VisualLoadHandle.IsValid())
	{
		VisualLoadHandle->CancelHandle();
		VisualLoadHandle.Reset();
	}
	
	// 캐시 미스면 아래 비동기로드 수행
	TArray<FSoftObjectPath> AssetsToLoad;
	
	if (InDataAsset->GetMesh().IsNull() == false)
		AssetsToLoad.Add(InDataAsset->GetMesh().ToSoftObjectPath());
	if (InDataAsset->GetAnimInstance().IsNull() == false)
		AssetsToLoad.Add(InDataAsset->GetAnimInstance().ToSoftObjectPath());
	
	if (AssetsToLoad.IsEmpty())
		return;
	
	VisualLoadHandle = UEDAssetManager::Get().LoadAssetsAsync(
		AssetsToLoad,
		FStreamableDelegate::CreateUObject(this, &AEDMonsterBase::OnVisualsLoaded)
	);
}

void AEDMonsterBase::OnVisualsLoaded()
{
	if (VisualLoadHandle.IsValid()) VisualLoadHandle.Reset();

	UEDMonsterDataAsset* DataAsset = GetDataAsset();
	if (IsValid(DataAsset) == false) return;

	USkeletalMesh* SkelMesh = DataAsset->GetMesh().Get();
	if (IsValid(SkelMesh) == false)
		return;
	// GetMesh()->SetSkeletalMesh(SkelMesh);
	GetMesh()->SetSkeletalMeshAsset(SkelMesh);
	UClass* AnimInstance = DataAsset->GetAnimInstance().Get();
	if (IsValid(AnimInstance) == false)
		return;
	GetMesh()->SetAnimInstanceClass(AnimInstance);
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] Visuals 로드 완료"), *GetName());
}

void AEDMonsterBase::HandleDeath()
{
	// Dead 상태로 전환 (OnRep_MonsterState로 클라이언트에 복제)
	MonsterState = EMonsterState::Dead;
	OnRep_MonsterState();
	// AIController BT 중단 및 Focus 해제
	AAIController* AIController = Cast<AAIController>(GetController());
	if (IsValid(AIController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] HandleDeath - AIController 없음"), *GetName());
		return;
	}
	
	if (IsValid(AIController->BrainComponent) == false)
		return;
	
	// 현재 Target 포커스 해제
	AIController->ClearFocus(EAIFocusPriority::Gameplay);
	// 이동 중지
	AIController->StopMovement();
	// BehaviorTree 중단
	AIController->BrainComponent->StopLogic(TEXT("Dead"));
	UE_LOG(LogTemp, Warning, TEXT("[%s] HandleDeath - 몬스터 사망"), *GetName());
	// GA_Death 어빌리티 발동
	FGameplayTagContainer DeathTag;
	DeathTag.AddTag(FEDGameplayTags::Get().State_Dead);
	AbilitySystemComponent->TryActivateAbilitiesByTag(DeathTag);
	// MonsterDeath 브로드 캐스트
	OnMonsterDeath.Broadcast();
	// 몬스터 시체에서 랜덤 루팅 아이템 스폰
	if (IsValid(InventoryComponent))
	{
		bool bResult = InventoryComponent->RequestInitializeRandomLoot();
		UE_LOG(LogTemp, Warning, TEXT("[%s] RandomLoot 요청: %s"), *GetName(), bResult ? TEXT("성공") : TEXT("실패(LootTable 없음)"));
	}
		
	
	// 20초 뒤에 몬스터 시체 처리 
	GetWorldTimerManager().SetTimer(
		DestroyMeshTimerHandle,
		FTimerDelegate::CreateWeakLambda(this,[this]()
		{
			Destroy();
		}),
		20.f, false);
}

void AEDMonsterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 서버에서만 처리하고 이미 죽었으면 호출 X
	if (HasAuthority() == false || MonsterState == EMonsterState::Dead)
		return;
	
	if (Data.NewValue <= 0.f)
		HandleDeath();
	
	// TODO: 히트 리액션 추가(필요시)
}