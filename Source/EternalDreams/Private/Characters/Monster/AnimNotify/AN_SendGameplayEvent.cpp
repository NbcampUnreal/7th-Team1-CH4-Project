// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Monster/AnimNotify/AN_SendGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

void UAN_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (EventTag.IsValid() == false)
		return;

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (IsValid(Owner) == false)
		return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner);
	if (ASI == nullptr)
		return;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (IsValid(ASC) == false)
		return;
	// ASC에 GameplayEvent 전달
	FGameplayEventData Payload;
	Payload.Instigator = Owner;
	ASC->HandleGameplayEvent(EventTag, &Payload);
}
