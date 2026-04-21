// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Projectile/ProjectileActor.h"

#include "AbilitySystemComponent.h"
#include "Projects.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Core/EDGameDataSubsystem.h"
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
	if (OtherActor==nullptr||OtherActor==this)
	{
		return;
	}
	
	
	IAbilitySystemInterface* AttackerASI=Cast<IAbilitySystemInterface>(GetOwner());
	if (AttackerASI==nullptr)
	{
		return;
	}
	TObjectPtr<UAbilitySystemComponent> AttackerASC=AttackerASI->GetAbilitySystemComponent();
	if (AttackerASC==nullptr)
	{
		return;
	}
	
	
	FGameplayEventData HitGameplayEventData;
		
	HitGameplayEventData.Target=OtherActor;
	AttackerASC->HandleGameplayEvent(FEDGameplayTags::Get().Event_SkillHit,&HitGameplayEventData);
	
	
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


