// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Projectile/ProjectileActor.h"

#include "AbilitySystemComponent.h"
#include "Projects.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Components/SphereComponent.h"
#include "Core/EDGameDataSubsystem.h"
#include "Core/EDPlayerState.h"
#include "Core/EDSkillDataSubsystem.h"
#include "Data/EDWeaponDataAsset.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Data/Types/EDPlayerTypes.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AProjectileActor::AProjectileActor()
{
	//레플리케이트
	bReplicates = true;
	SetReplicateMovement(true);
	
	//루트 스피어 컴포넌트로 충돌 판정을 진행
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(SphereComponent);
	SphereComponent->SetCollisionProfileName(TEXT("BlockAll"));
	SphereComponent->OnComponentHit.AddDynamic(this, &AProjectileActor::OnProjectileHit);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECR_Ignore);
	//스태틱 메시 설정 및 콜리전 해제
	ProjectileStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	ProjectileStaticMesh->SetupAttachment(SphereComponent);
	ProjectileStaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	ProjectileStaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility,ECR_Ignore);
	//PMC 컴포넌트 생성
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(SphereComponent);
	//PMC 설정. 디폴트 초기속도 : 1000.f
	ProjectileMovement->InterpLocationTime = 0.05f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;

}

void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp,Warning,TEXT("Projectile"));
	//로드된 StaticMesh 적용
	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
	if (EDGameplayDataSubsystem)
	{
		SetStaticMeshId(FPrimaryAssetId(
			*UEnum::GetDisplayValueAsText(EPlayerDataType::WeaponData).ToString(),
			*UEnum::GetDisplayValueAsText(EWeaponNameType::Arrow).ToString()
			));
	}
	
	//Projectile 활성화
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileMovement->Activate(true);
	ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,this,&AProjectileActor::LifeTimeEnd,5.0f,false);
	
	if (GetOwner()==nullptr)
	{
		return;
	}
	
	SphereComponent->MoveIgnoreActors.Add(this);
	SphereComponent->MoveIgnoreActors.Add(GetOwner());
	TArray<AActor*> OwnerAttachedActors;
	GetOwner()->GetAttachedActors(OwnerAttachedActors);
	if (OwnerAttachedActors.Num() > 0)
	{
		for (AActor* Actor : OwnerAttachedActors)
		{
			SphereComponent->MoveIgnoreActors.Add(Actor);
		}
	}
	
		
	//플레이어의 화살 생성 시점 변수를 캡처
	AEDPlayerCharacter* Player=Cast<AEDPlayerCharacter>(GetOwner());
	UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
	if (!PlayerASC)
	{
		return;
	}
	const UEDPlayerAttributeSet* PlayerAttributeSet = Cast<UEDPlayerAttributeSet>(PlayerASC->GetAttributeSet(UEDPlayerAttributeSet::StaticClass()));
	if (PlayerAttributeSet)
	{
		CapturePlayerStrength= PlayerAttributeSet->GetStrength();
		CapturePlayerDexterity= PlayerAttributeSet->GetDexterity();
		CapturePlayerIntelligence= PlayerAttributeSet->GetIntelligence();
	}
}

void AProjectileActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AProjectileActor,StaticMeshId);
}

void AProjectileActor::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (GetWorld()==nullptr)
	{
		return;
	}
	if (HasAuthority()==false)
	{
		return;
	}
	if (OtherActor==nullptr||OtherActor==this||GetOwner()==nullptr)
	{
		return;
	}
	
	AEDPlayerCharacter* HittedPlayer=Cast<AEDPlayerCharacter>(OtherActor);
	AEDPlayerCharacter* AttackedPlayer=Cast<AEDPlayerCharacter>(GetOwner());
	if (HittedPlayer&&AttackedPlayer&&HittedPlayer->GetController()&&AttackedPlayer->GetController())
	{
		AEDPlayerState* HittedPlayerState=HittedPlayer->GetController()->GetPlayerState<AEDPlayerState>();
		AEDPlayerState* AttackedPlayerState=AttackedPlayer->GetController()->GetPlayerState<AEDPlayerState>();
		if (HittedPlayerState&&AttackedPlayerState)
		{
			//같은 팀인 경우 GE 적용하지 않음
			if (HittedPlayerState->TeamId==AttackedPlayerState->TeamId)
			{
				Destroy();
				return;
			}
		}
		
	}
	
	
	
	
	//맞은 적의 ASI, ASC를 가져온다.
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(OtherActor);
	if (TargetASI == nullptr)
	{
		return;
	}
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (TargetASC == nullptr)
	{
		return;
	}

	AEDPlayerCharacter* Player=Cast<AEDPlayerCharacter>(GetOwner());
	
	
	UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
	if (!PlayerASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = PlayerASC->MakeEffectContext();
	Context.AddSourceObject(Player); // 소스 오브젝트는 현재 캐릭터(Avatar)

	FGameplayEffectSpecHandle SpecHandle = PlayerASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
	if (SpecHandle.IsValid() )
	{
		//AssetTag 로 검색
		
		const UEDSkillDataSubsystem* EDSkillDataSubsystem=UEDSkillDataSubsystem::Get(GetWorld());
		
		if (ProjectileSkillTag==FGameplayTag::EmptyTag||!IsValid(EDSkillDataSubsystem))
		{
			return;
		}
		
		const FSkillMulStatus* SkillMulStaus =EDSkillDataSubsystem->GetSkillData(ProjectileSkillTag);
		
		if (SkillMulStaus==nullptr)
		{
			return;
		}
		
		
		float SkillFinalDamage =
			CapturePlayerStrength * SkillMulStaus->DamageStrengthMultiplier +
			CapturePlayerDexterity * SkillMulStaus->DamageDexterityMultiplier +
			CapturePlayerIntelligence * SkillMulStaus->DamageIntelligenceMultiplier
		;

		SpecHandle.Data->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_Damage, SkillFinalDamage);
		PlayerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
	
	UE_LOG(LogTemp,Warning,TEXT("%s"),*OtherActor->GetName());
	Destroy();
}



void AProjectileActor::LifeTimeEnd()
{
	Destroy();
}

void AProjectileActor::ApplyWeaponMesh()
{
	if (!StaticMeshId.IsValid())
    	{
    		ProjectileStaticMesh->SetStaticMesh(nullptr);
    	}
    	
    	const UEDGameDataSubsystem* EDGameplayDataSubsystem=UEDGameDataSubsystem::Get(GetWorld());
    	if (!EDGameplayDataSubsystem)
    	{
    		return;
    	}
    	
    	UEDWeaponDataAsset* Weapon = 
    			EDGameplayDataSubsystem->GetData<UEDWeaponDataAsset>(StaticMeshId
    				);
    		
    	if (IsValid(Weapon))
    	{
    		ProjectileStaticMesh->SetStaticMesh(Weapon->WeaponStaticMesh.LoadSynchronous());
    	}
}


