#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Core/EDInventoryTypes.h"
#include "EDLootInteractionComponent.generated.h"

class AActor;
class APlayerController;
class UEDInventoryComponent;
class UEDUIManageSubsystem;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEDLootTransferResult, bool, EEDInventoryActionFailure);

/**
 * 플레이어의 루팅 상호작용을 중개하는 컴포넌트
 * 현재 루팅 대상, 루팅 패널 열기/닫기, 서버 루팅 전송을 담당
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UEDLootInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEDLootInteractionComponent();

	// 현재 플레이어가 상호작용 가능한 루팅 대상 액터를 설정
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetCurrentLootTarget(AActor* InLootTarget);

	// 현재 루팅 대상 액터를 해제
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void ClearCurrentLootTarget(AActor* InLootTarget = nullptr);

	// 현재 루팅 대상 액터를 반환
	UFUNCTION(BlueprintPure, Category = "Loot")
	AActor* GetCurrentLootTarget() const;

	// 루팅 패널 토글 입력을 처리
	void HandleToggleLootPanel();

	// 루팅 패널에서 요청한 아이템 이동을 서버로 전달
	void RequestLootTransfer(UEDInventoryComponent* FromInventory, int32 FromSlotIndex, int32 Quantity);

	// 루팅 전송 결과를 패널에 전달
	FOnEDLootTransferResult OnLootTransferResult;

protected:
	UFUNCTION(Server, Reliable)
	void ServerRequestLootTransfer(UEDInventoryComponent* FromInventory, int32 FromSlotIndex, int32 Quantity);

	UFUNCTION(Client, Reliable)
	void ClientNotifyLootTransferResult(bool bSuccess, EEDInventoryActionFailure Failure);

private:
	// 현재 루팅 패널을 열 수 있는지 확인
	bool CanOpenLootPanel() const;

	// 현재 루팅 대상 액터에서 인벤토리 컴포넌트 가져오기
	UEDInventoryComponent* ResolveCurrentLootInventoryComponent() const;

	// 현재 루팅 대상을 기준으로 루팅 패널 열기
	void OpenLootPanelForCurrentTarget();

	// 현재 열려 있는 루팅 패널 닫기
	void CloseLootPanelIfOpen();

	// 소유 플레이어 컨트롤러를 반환
	APlayerController* GetOwningPlayerController() const;

	// UI 관리 서브시스템을 반환
	UEDUIManageSubsystem* GetUIManageSubsystem() const;

	// 현재 플레이어가 상호작용 가능한 루팅 대상 액터
	TWeakObjectPtr<AActor> CurrentLootTarget;
};
