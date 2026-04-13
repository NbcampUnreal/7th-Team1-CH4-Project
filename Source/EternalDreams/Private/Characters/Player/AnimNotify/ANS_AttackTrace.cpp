// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/AnimNotify/ANS_AttackTrace.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Characters/Player/EDPlayerCharacter.h"
#include "Characters/Player/Weapon/EDWeapon.h"
#include "Kismet/KismetSystemLibrary.h"

UANS_AttackTrace::UANS_AttackTrace()
{
	Owner = nullptr;
	bShowDebug = false;
}

void UANS_AttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	//Owner, Weapon, WeaponMesh, AttackerASI, AttackerASC 캐싱
	Owner = MeshComp->GetOwner();
	if (Owner == nullptr)
	{
		return;
	}
	Owner->GetAttachedActors(AttachedActors);
	if (!AttachedActors.IsEmpty())
	{
		Weapon = Cast<AEDWeapon>(AttachedActors[0]);
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
	
	
	//소켓 위치로 변수 초기화
	CurrentAttackSocketLocation = WeaponMesh->GetSocketTransform(SocketName, RTS_World).
												  GetLocation();
	PresentAttackSocketLocation = WeaponMesh->GetSocketTransform(SocketName, RTS_World).
												  GetLocation();
	
	
}

void UANS_AttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!IsValid(WeaponMesh)||AttackerASC==nullptr)
	{
		return;
	}

	CurrentAttackSocketLocation = WeaponMesh->GetSocketTransform(SocketName, RTS_World).
	                                              GetLocation();

	//Sphere Trace
	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Owner);
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		Owner->GetWorld(),
		PresentAttackSocketLocation,
		CurrentAttackSocketLocation,
		TraceRadius,
		//Pawn만 Trace 처리
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false, // bTraceComplex
		ActorsToIgnore,
		DebugType,
		HitResults,
		true, // bIgnoreSelf
		FLinearColor::Red, // TraceColor
		FLinearColor::Green, // TraceHitColor
		2.0f // DrawTime
	);
	//이전 소켓의 위치를 갱신
	PresentAttackSocketLocation = CurrentAttackSocketLocation;
	//피격되지 않았으면 Early Return
	if (!bHit)
	{
		return;
	}
	for (const FHitResult& HitResult : HitResults)
	{
		HittedActor=HitResult.GetActor();
		//1번 공격시 이미 피격당한 목록에 있다면 중복 타격 방지
		if (!HittedActor||HittedCharacterArray.Contains(HittedActor))
		{
			continue;
		}
		//피격당한 캐릭터를 메인캐릭터로 형변환
		HittedPlayer=Cast<AEDPlayerCharacter>(HittedActor);
		if (HittedPlayer == nullptr)
		{
			continue;
		}
		//피격당한 목록에 추가
		HittedCharacterArray.Add(HittedPlayer);
		
		//맞은 적의 ASI, ASC를 가져온다.
		IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HittedPlayer);
		if (TargetASI == nullptr)
		{
			continue;
		}
		UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
		if (TargetASC==nullptr)
		{
			continue;
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
		
		//넉백 추가
		FVector KnockBackDirection;
		FVector2D KnockBackDirection2D;
		KnockBackDirection=(HittedPlayer->GetActorLocation()-Owner->GetActorLocation());
		KnockBackDirection2D=FVector2D(KnockBackDirection.X,KnockBackDirection.Y);
		//정규화는 XY만 필요할 경우 2D에서 해야 연산량 감소
		KnockBackDirection2D.Normalize();
		KnockBackDirection2D*=LaunchPowerXY;
		KnockBackDirection=FVector(KnockBackDirection2D.X,KnockBackDirection2D.Y,LaunchPowerZ);
		HittedPlayer->LaunchCharacter(KnockBackDirection,true,true);
	}
}

void UANS_AttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	//피격당한 목록을 비워준다.
	HittedCharacterArray.Empty();
}
