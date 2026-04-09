// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/ZoneDetectorComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "GameplayEffect.h"

UZoneDetectorComponent::UZoneDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); // 컴포넌트 리플리케이션 켜기 
}

void UZoneDetectorComponent::EnterRestrictedArea(TSubclassOf<UGameplayEffect> EffectClass)
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!ASC) return;

	OverlappingZoneCount++;

	if (OverlappingZoneCount == 1 && EffectClass)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddInstigator(GetOwner(), GetOwner());

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);

		if (SpecHandle.IsValid())
		{
			RestrictedAreaEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			UE_LOG(LogTemp, Warning, TEXT("[Server] 금지구역 진입 (중첩: %d)"), OverlappingZoneCount);
		}
	}

	// 💡 서버에서 처리 완료 후, 해당 플레이어의 클라이언트 화면에 디버그 출력 명령
	Client_ShowZoneDebugMessage(true, OverlappingZoneCount);
}

void UZoneDetectorComponent::ExitRestrictedArea()
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!ASC) return;

	OverlappingZoneCount--;

	if (OverlappingZoneCount <= 0)
	{
		OverlappingZoneCount = 0;

		if (RestrictedAreaEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(RestrictedAreaEffectHandle);
			RestrictedAreaEffectHandle.Invalidate();
			UE_LOG(LogTemp, Warning, TEXT("[Server] 금지구역 이탈"));
		}
	}

	// 클라이언트 화면에 이탈 디버그 출력 명령
	Client_ShowZoneDebugMessage(false, OverlappingZoneCount);
}

// 실제 클라이언트의 화면에서 실행되는 부분 (_Implementation을 붙여야 함)
void UZoneDetectorComponent::Client_ShowZoneDebugMessage_Implementation(bool bIsEntering, int32 CurrentZoneCount)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	if (bIsEntering)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				-1, 3.0f, FColor::Red,
				FString::Printf(TEXT("[클라이언트] 금지구역 진입! (중첩: %d)"), CurrentZoneCount));

		FString Msg = FString::Printf(TEXT("금지구역 진입!\n카운트: %d"), CurrentZoneCount);
		DrawDebugString(GetWorld(), OwnerActor->GetActorLocation() +
		                FVector(0, 0, 300), Msg, nullptr, FColor::Red, 3.0f, true, 1.2f);
	}
	else
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				-1, 3.0f, FColor::Green,
				FString::Printf(TEXT("[클라이언트] 금지구역 이탈! (남은 중첩: %d)"), CurrentZoneCount));

		FString Msg = FString::Printf(TEXT("금지구역 이탈!\n카운트: %d"), CurrentZoneCount);
		DrawDebugString(GetWorld(), OwnerActor->GetActorLocation() +
		                FVector(0, 0, 100), Msg, nullptr, FColor::Green, 3.0f, true, 1.2f);
	}
}
