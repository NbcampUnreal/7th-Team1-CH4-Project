// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/ZoneDetectorComponent.h"
#include "Data/GameplayTag/EDGameplayTags.h"  // 네이티브 게임플레이 태그 선언 (ini에서 변경)
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

    // 첫 번째 금지 구역 진입 시에만 상태를 설정하고 이펙트를 적용
    if (OverlappingZoneCount == 1)
    {
       // Tag부착 '금지구역 상태'
       ASC->AddLooseGameplayTag(FEDGameplayTags::Get().State_Player_RestrictedArea);

       // 이펙트적용 (`GE_RestricTimer`)
       if (EffectClass)
       {
          FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
          ContextHandle.AddInstigator(GetOwner(), GetOwner());
          FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, 1.0f, ContextHandle);

          if (SpecHandle.IsValid())
          {
             // 적용된 이펙트의 Handle을 나중에 제거하기 위해 저장합니다.
             RestrictedAreaEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
          }
       }
       UE_LOG(LogTemp, Warning, TEXT("[Server] 금지구역 진입 (중첩: %d) / C++ 태그 'State.Player.RestrictedArea' 부여됨"), OverlappingZoneCount);
    }

    Client_ShowZoneDebugMessage(true, OverlappingZoneCount);
}

void UZoneDetectorComponent::ExitRestrictedArea()
{
    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
    if (!ASC) return;

    OverlappingZoneCount--;

    // 금지구역카운트0일때 상태를 해제하고 이펙트를 제거
    if (OverlappingZoneCount <= 0)
    {
       OverlappingZoneCount = 0;

       // '금지구역 상태' Tag 제거
       ASC->RemoveLooseGameplayTag(FEDGameplayTags::Get().State_Player_RestrictedArea);

       // 저장해둔 Handle을 이용해 이펙트 제거
       if (RestrictedAreaEffectHandle.IsValid())
       {
          ASC->RemoveActiveGameplayEffect(RestrictedAreaEffectHandle);
          RestrictedAreaEffectHandle.Invalidate(); // 핸들 초기화
       }
       UE_LOG(LogTemp, Warning, TEXT("[Server] 금지구역 이탈 / C++ 태그 제거됨 및 로직 이펙트 제거됨"));
    }

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
