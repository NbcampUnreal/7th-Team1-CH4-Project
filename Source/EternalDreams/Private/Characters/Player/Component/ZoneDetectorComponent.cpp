// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/Component/ZoneDetectorComponent.h"
#include "Data/GameplayTag/EDGameplayTags.h"  // 네이티브 게임플레이 태그 선언 (ini에서 변경)
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "GameplayEffect.h"

//UI용 헤더
#include "UI/HUD/EDTimerWidget.h"
#include "Characters/Player/GAS/EDPlayerAttributeSet.h"
#include "Kismet/GameplayStatics.h"


UZoneDetectorComponent::UZoneDetectorComponent()
{
	//PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bCanEverTick = true;
	//PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	SetIsReplicatedByDefault(true); // 컴포넌트 리플리케이션 켜기 
}

void UZoneDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ASC에서 태그가 추가/제거되는 것을 감시하는 리스너 등록
	/*if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		FGameplayTag ZoneTag = FEDGameplayTags::Get().State_Player_RestrictedArea;
		ASC->RegisterGameplayTagEvent(ZoneTag, EGameplayTagEventType::NewOrRemoved)
		   .AddUObject(this, &UZoneDetectorComponent::OnRestrictedAreaTagChanged);
	}*/
	
	// 서버일 때만 안전하게 ASC의 동기화 모드를 변경합니다.
	if (GetOwner()->HasAuthority())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
		{
			// TODO: 컴포넌트가 Mixed 모드로 세팅 (현재 Minimal이라서 플레이어에 위젯이 안보임)
			ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
			UE_LOG(LogTemp, Log, TEXT("ZoneDetector: 캐릭터의 ASC 모드를 Mixed로 강제 변경했습니다."));
		}
	}
	
	
}

/*
void UZoneDetectorComponent::OnRestrictedAreaTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    // 데디케이티드 서버에서는 UI를 생성하면 안 되므로 예외 처리
    if (GetWorld()->GetNetMode() == NM_DedicatedServer) return;

    if (NewCount > 0) // 금지구역 진입 (태그 생김)
    {
        if (!TimerWidget && TimerWidgetClass)
        {
            // 위젯 생성 및 화면 바닥(-1 레이어)에 추가
            TimerWidget = CreateWidget<UEDTimerWidget>(GetWorld(), TimerWidgetClass);
            if (TimerWidget)
            {
                TimerWidget->AddToViewport(-1);
                SetComponentTickEnabled(true); // 머리 위 추적을 위해 틱 켜기
            }
        }
    }
    else // 금지구역 이탈 (태그 사라짐)
    {
        if (TimerWidget)
        {
            TimerWidget->RemoveFromParent();
            TimerWidget = nullptr;
            SetComponentTickEnabled(false); // 틱 다시 끄기 (최적화)
        }
    }
}
*/

/*
void UZoneDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (TimerWidget && GetOwner())
    {
        // 1. 남은 시간 업데이트
        if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
        {
            float TimeRemaining = ASC->GetNumericAttribute(UEDPlayerAttributeSet::GetSurvivalTimeAttribute());
            TimerWidget->UpdateTimeText(TimeRemaining);
        }

        // 2. 머리 위 좌표 추적
        // 내 화면을 담당하는 로컬 플레이어 컨트롤러를 가져옴
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            // 캐릭터 위치에서 Z축으로 약간 올려서 머리 위 지정 (숫자는 120.f 등 적절히 조절)
            FVector WorldLocation = GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 200.f);
            FVector2D ScreenLocation;

            // 캐릭터가 내 화면(카메라 앞)에 있을 때만 위치를 갱신
            if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenLocation))
            {
                TimerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
                TimerWidget->SetPositionInViewport(ScreenLocation);
            	
            	TimerWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
            }
            else
            {
                // 캐릭터가 내 등 뒤에 있거나 카메라 밖이면 UI를 숨김
                TimerWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }
}
*/

void UZoneDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 데디케이티드 서버는 UI 연산을 할 필요가 없으므로 즉시 종료
    if (GetWorld()->GetNetMode() == NM_DedicatedServer) return;

    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
    if (!ASC) return;

    // 1. 현재 내 캐릭터에 금지구역 태그가 있는지 '직접' 확인 (가장 확실한 방법)
    FGameplayTag ZoneTag = FEDGameplayTags::Get().State_Player_RestrictedArea;
    bool bIsInZone = ASC->HasMatchingGameplayTag(ZoneTag);

    if (bIsInZone)
    {
        // 2. 금지구역인데 아직 위젯이 안 만들어졌다면 생성
        if (!TimerWidget && TimerWidgetClass)
        {
            TimerWidget = CreateWidget<UEDTimerWidget>(GetWorld(), TimerWidgetClass);
            if (TimerWidget)
            {
                TimerWidget->AddToViewport(-1);
            }
        }

        // 3. 위젯 갱신 및 머리 위 좌표 추적
        if (TimerWidget)
        {
            float TimeRemaining = ASC->GetNumericAttribute(UEDPlayerAttributeSet::GetSurvivalTimeAttribute());
            TimerWidget->UpdateTimeText(TimeRemaining);

            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC)
            {
                FVector WorldLocation = GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 300.f);
                FVector2D ScreenLocation;

                if (PC->ProjectWorldLocationToScreen(WorldLocation, ScreenLocation))
                {
                    TimerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
                    TimerWidget->SetPositionInViewport(ScreenLocation);
                    TimerWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
                }
                else
                {
                    TimerWidget->SetVisibility(ESlateVisibility::Collapsed);
                }
            }
        }
    }
    else
    {
        // 4. 금지구역 밖으로 나갔는데(태그 없음) 위젯이 아직 켜져있다면 파괴
        if (TimerWidget)
        {
            TimerWidget->RemoveFromParent();
            TimerWidget = nullptr;
        }
    }
}

void UZoneDetectorComponent::EnterRestrictedArea(TSubclassOf<UGameplayEffect> EffectClass)
{
	if (!GetOwner()->HasAuthority()) return;
	
    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
    if (!ASC) return;

    OverlappingZoneCount++;

    // 첫 번째 금지 구역 진입 시에만 상태를 설정하고 이펙트를 적용
    if (OverlappingZoneCount == 1)
    {
       // GE_RestricTimer 이펙트에서 Tag부착 '금지구역 상태'

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
	if (!GetOwner()->HasAuthority()) return;
	
    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
    if (!ASC) return;

    OverlappingZoneCount--;

    // 금지구역카운트0일때 상태를 해제하고 이펙트를 제거
    if (OverlappingZoneCount <= 0)
    {
       OverlappingZoneCount = 0;

    	//GE_RestricTimer 이펙트에서 Tag제거 '금지구역 상태'
    	
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
