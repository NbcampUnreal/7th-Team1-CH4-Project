// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/AnimNotify/ANS_MonsterAttackTrace.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Characters/Base/GAS/EDBaseAttributeSet.h"
#include "Data/EDMonsterDataAsset.h"
#include "Data/GameplayTag/EDGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"

UANS_MonsterAttackTrace::UANS_MonsterAttackTrace()
{
}

void UANS_MonsterAttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(MeshComp->GetOwner());
	if (IsValid(Monster) == false)
		return;
	
	FVector Loc = MeshComp->GetSocketTransform(SocketName, RTS_World).GetLocation();
	PrevLocationMap.Add(MeshComp, Loc);
	HitMap.Add(MeshComp, {});
}

void UANS_MonsterAttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	// 데미지 판정은 서버에서만
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(MeshComp->GetOwner());
	if (IsValid(Monster) == false || Monster->HasAuthority() == false)
		return;
	
	UAbilitySystemComponent* MonsterASC = Monster->GetAbilitySystemComponent();
	if (IsValid(MonsterASC) == false)
		return;
	
	FVector* Prev = PrevLocationMap.Find(MeshComp);
	TArray<TWeakObjectPtr<AActor>>* HitArray = HitMap.Find(MeshComp);
	if (Prev == nullptr || HitArray == nullptr)
		return;
	
	// 소켓 위치 갱신
	FVector Curr = MeshComp->GetSocketTransform(SocketName, RTS_World).GetLocation();
	// 충돌 결과 배열
	TArray<FHitResult> HitResults;
	// 무시할 액터 == Monster
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Monster);
	// 디버그 타입 bShowDebug의 bool값에 따라 지속 또는 없음
	EDrawDebugTrace::Type DebugType = bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	// 스피어 멀티 트레이스 실행 및 bool 값 반환
	bool bHit = UKismetSystemLibrary::SphereTraceMulti(
		Monster->GetWorld(),
		*Prev,
		Curr,
		TraceRadius,
		UEngineTypes::ConvertToTraceType(ECC_Pawn),
		false,
		ActorsToIgnore,
		DebugType,
		HitResults,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		2.0f
		);
	// 트레이스 중 PrevSocketLocation 위치 갱신
	*Prev = Curr;
	
	if (bHit == false)
		return;
	// 충돌 결과 순회
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor) == false || HitArray->Contains(HitActor))
			continue;
		IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(HitActor);
		if (TargetASI == nullptr)
			continue;
		
		UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
		if (IsValid(TargetASC) == false)
			continue;
		
		// 예외 처리 통과시 충돌 캐릭터 배열에 추가
		HitArray->Add(HitActor);
		// DataAsset의 Atk 참조 Monster의 Atk은 GA에 없기 때문
		const float Atk = IsValid(Monster->GetDataAsset()) ?  Monster->GetDataAsset()->GetStat().Atk : 0.f;
		
		FGameplayEffectContextHandle Context = MonsterASC->MakeEffectContext();
		Context.AddSourceObject(Monster);
		FGameplayEffectSpecHandle SpecHandle = MonsterASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
		
		if (SpecHandle.IsValid() == false)
			continue;
		// Data_Damage에 -Atk 수치 입력 및 TargetASC(Player)에게 GE로 데미지 적용
		SpecHandle.Data->SetSetByCallerMagnitude(FEDGameplayTags::Get().Data_Damage, Atk);
		MonsterASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		
		UE_LOG(LogTemp, Warning, TEXT("[MonsterAttackTrace] 플레이어 HP: %.1f"),
			TargetASC->GetNumericAttribute(UEDBaseAttributeSet::GetHealthAttribute()));
	}
}

void UANS_MonsterAttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	// 공격 완료 후 정리
	PrevLocationMap.Remove(MeshComp);
	HitMap.Remove(MeshComp);
}
