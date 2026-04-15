// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AnimNotify/ANS_HitScanAttack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/Weapon/EDWeapon.h"
#include "Kismet/KismetSystemLibrary.h"

UANS_HitScanAttack::UANS_HitScanAttack()
{
	Owner = nullptr;
}

void UANS_HitScanAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                     float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	//Owner, Weapon, WeaponMesh, AttackerASI, AttackerASC 캐싱
	Owner = MeshComp->GetOwner();
	if (Owner == nullptr)
	{
		return;
	}
	TArray<AActor*> AttachedActors;
	Owner->GetAttachedActors(AttachedActors);
	//오른손 무기 전용이므로 RWeaponSocketName에 붙어있는 Actor만 가져온다.
	if (!AttachedActors.IsEmpty())
	{
		for (AActor* Actor:AttachedActors)
		{
			AEDWeapon* WeaponActor=Cast<AEDWeapon>(Actor);
			if (!IsValid(WeaponActor))
			{
				continue;
			}
			if (WeaponActor->GetStaticMesh()!=nullptr)
			{
				Weapon = WeaponActor;
				break;
			}
		}
	}
	if (Weapon == nullptr)
	{
		return;
	}
	WeaponMesh=Weapon->GetComponentByClass<UStaticMeshComponent>();
	if (WeaponMesh==nullptr)
	{
		return;
	}
	AttackerASI=Cast<IAbilitySystemInterface>(Owner);
	if (AttackerASI==nullptr)
	{
		return;
	}
	AttackerASC=AttackerASI->GetAbilitySystemComponent();
	if (AttackerASC==nullptr)
	{
		return;
	}
	
	SocketLocation=WeaponMesh->GetSocketTransform(SocketName, RTS_World).GetLocation();
	SocketDirection=MeshComp->GetOwner()->GetActorForwardVector();
	//발사체의 위치와 방향을 세팅합니다.

	
	FVector SpawnLocation = SocketLocation+SocketDirection*(AttackDistance/2);
	
	//기존 바라보는 방향대로 타겟
	FRotator SpawnRotation=MeshComp->GetOwner()->GetActorRotation();
	
	
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	AActor* WarningActor=MeshComp->GetWorld()->SpawnActorDeferred<AActor>(WarningActorClass,SpawnTransform);
	
	if (IsValid(WarningActor))
	{
		WarningActor->SetOwner(MeshComp->GetOwner());
		WarningActor->FinishSpawning(SpawnTransform);
		WarningActor->SetLifeSpan(TotalDuration);
		WarningActor->SetActorScale3D(FVector(1.f,1.f,AttackDistance));
	}
	
	
}

void UANS_HitScanAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (!IsValid(this)) 
	{
		return;
	}

	
	
	if (!FinalShootActorClass)
	{
		return;
	}
	AActor* ShootActor=MeshComp->GetWorld()->SpawnActorDeferred<AActor>(FinalShootActorClass,SpawnTransform);
	
	if (IsValid(ShootActor))
	{
		ShootActor->SetOwner(MeshComp->GetOwner());
		ShootActor->SetLifeSpan(ShootActorLifeSpan);
		ShootActor->FinishSpawning(SpawnTransform);
		ShootActor->SetActorScale3D(FVector(1.f,1.f,AttackDistance));
	}
	
	//Sphere Trace
	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	FVector StartLocation= SocketLocation;
	FVector EndLocation=StartLocation+SocketDirection*AttackDistance;
	
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		Owner->GetWorld(),
		StartLocation,
		EndLocation,
		//Pawn만 Trace 처리
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, // bTraceComplex
		ActorsToIgnore,
		DebugType,
		HitResult,
		true, // bIgnoreSelf
		FLinearColor::Red, // TraceColor
		FLinearColor::Green, // TraceHitColor
		2.0f // DrawTime
	);
	
	if (!bHit)
	{
		return;	
	}
	
	AActor* HittedActor=HitResult.GetActor();

	if (!HittedActor)
	{
		return;
	}
	//피격당한 캐릭터를 메인캐릭터로 형변환
	AEDPlayerCharacter* HittedPlayer=Cast<AEDPlayerCharacter>(HittedActor);
	if (HittedPlayer == nullptr)
	{
		return;
	}
	
		
	//맞은 적의 ASI, ASC를 가져온다.
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HittedPlayer);
	if (TargetASI == nullptr)
	{
		return;
	}
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (TargetASC==nullptr)
	{
		return;
	}
	if (AttackerASC==nullptr)
	{
		return;
	}
	
	//GE 적용
	FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
	Context.AddSourceObject(Owner);

	FGameplayEffectSpecHandle SpecHandle = AttackerASC->MakeOutgoingSpec(
		DamageEffectClass, 1.0f, Context);

	if (SpecHandle.IsValid())
	{
		AttackerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}

}
