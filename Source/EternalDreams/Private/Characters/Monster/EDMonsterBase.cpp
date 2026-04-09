// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/EDMonsterBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Characters/Monster/EDMonsterAnimInstance.h"
#include "Data/EDMonsterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AEDMonsterBase::AEDMonsterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// 몬스터 GE는 서버만 가지고있고 계산하기 때문에 Minimal로 설정
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	// Idle상태로 시작
	MonsterState = EMonsterState::Idle;
	
	BaseAttributeSet = CreateDefaultSubobject<UEDBaseAttributeSet>(TEXT("BaseAttributeSet"));
}

// Called when the game starts or when spawned
void AEDMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsValid(AbilitySystemComponent) == false)
		return;
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	
	OriginLocation = GetActorLocation();
	
	if (HasAuthority() == false)
		return;
	if (IsValid(DataAsset) == false)
		return;
	
	InitializeFromDataAsset(DataAsset);
	
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
	DataAsset = InDataAsset;
	
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
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr)
		return;
	MoveComp->MaxWalkSpeed = Stat.MoveSpeed;
	MoveComp->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
		
	// TODO : Mesh, AnimInstance - DataAsset에 getter 추가 후 비동기 로드
}

void AEDMonsterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEDMonsterBase, MonsterState);
}

void AEDMonsterBase::OnRep_MonsterState()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] Monster State: %d"), *GetName(), (int32)MonsterState);
	// TODO: AnimInstance에 전달
	UEDMonsterAnimInstance* Anim = Cast<UEDMonsterAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(Anim) == false)
		return;
	
	Anim->SetMonsterState(MonsterState);
}


