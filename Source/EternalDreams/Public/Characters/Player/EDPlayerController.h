// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "EDPlayerController.generated.h"

struct FInputActionValue;
class AEDCameraActor;
class AEDCursorActor;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;
class UEDCraftingInteractionComponent;
class UEDLootInteractionComponent;

DECLARE_DELEGATE_OneParam(FOnOtherInput,FInputActionValue);
DECLARE_MULTICAST_DELEGATE(FOnCraftInputTriggered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadedComplete);

/**
 * 플레이어 컨트롤러 클래스
 */
UCLASS()
class ETERNALDREAMS_API AEDPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

	AEDPlayerController();

	// -------------------------------------------------------
	// IGenericTeamAgentInterface
	// -------------------------------------------------------

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	virtual void BeginPlay() override;
		// 작성자 : 김동주
    	// Enhanced Input 액션을 실제 처리 함수에 바인딩
	virtual void SetupInputComponent() override;

public:
	FOnCraftInputTriggered& GetOnCraftInputTriggered() { return OnCraftInputTriggered; }

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerNickname(const FString& InNickname);
	
	//현석: 로드 완료시 로딩창 삭제
	UPROPERTY(BlueprintAssignable)
	FOnLoadedComplete OnAllLoadCompleted;
	
	UFUNCTION(Client,Reliable)
	void ClientRPC_LoadComplete();
	
#pragma region Input UI
	// UI 입력 전용 매핑 컨텍스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI")
	TObjectPtr<UInputMappingContext> UIInputMappingContext = nullptr;
	
	// 루팅 인벤토리 패널 열기/닫기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI")
	TObjectPtr<UInputAction> ToggleLootInventoryAction = nullptr;

	// 아이템 제작 패널 열기/닫기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI")
	TObjectPtr<UInputAction> ToggleCraftPanelAction = nullptr;

	// ESC 입력 처리용 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|UI")
	TObjectPtr<UInputAction> UIBackAction = nullptr;
#pragma endregion 김동주

	// -------------------------------------------------------
	// 사망 / 부활 RPC
	// -------------------------------------------------------

	/**
	 * [서버→클라이언트] 사망 직후 호출.
	 * 사망 오버레이를 뷰포트에 띄우고 카운트다운 시작.
	 * bCanRespawn=false면 Eliminated 표시로 전환.
	 */
	UFUNCTION(Client, Reliable, Category = "ED|Death")
	void ClientOnPlayerDied(float CountdownSeconds, bool bCanRespawn);

	/**
	 * [서버→클라이언트] 사망 후 카운트다운 경과 시 호출.
	 * RespawnZoneSelect 위젯을 뷰포트에 띄운다. 유저가 구역 선택 전까지 계속 관전.
	 */
	UFUNCTION(Client, Reliable, Category = "ED|Death")
	void ClientOpenZoneSelectWidget();

	/**
	 * [클라이언트→서버] 유저가 ZoneSelectWidget에서 구역 선택 후 호출.
	 * 서버가 DesiredZoneId 갱신 후 RestartPlayer를 실행한다.
	 */
	UFUNCTION(Server, Reliable, Category = "ED|Death")
	void Server_RequestRespawn(int32 SelectedZoneId);

	// -------------------------------------------------------
	// 매치 종료 RPC
	// -------------------------------------------------------

	/**
	 * [서버→클라이언트] 매치 종료 시 호출.
	 * TeamRankings: index 0=1등, 1=2등, ... 순서로 정렬된 팀 ID 배열.
	 * UIManager의 Panel_MatchResult를 열고 SetResult로 전달.
	 */
	UFUNCTION(Client, Reliable, Category = "ED|Match")
	void ClientShowMatchResult(const TArray<int32>& TeamRankings);

public:
#pragma region Input Player
	//IMC_Player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputMappingContext> PlayerInputMappingContext=nullptr;
  // IA_Player
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> MoveAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> LookAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> BasicAttackAction=nullptr;

	// 제작 실행 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> CraftItemAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> QSkillAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> ESkillAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Player")
	TObjectPtr<UInputAction> SpaceSkillAction=nullptr;
#pragma endregion
#pragma region Input Camera
	//IMC_Camera
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Camera")
	TObjectPtr<UInputMappingContext> CameraInputMappingContext=nullptr;
	// IA_Camera
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Camera")
	TObjectPtr<UInputAction> WheelAction=nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input|Camera")
	TObjectPtr<UInputAction> KeyboardCAction=nullptr;
#pragma endregion
#pragma region Input Bindings
public:
	UFUNCTION()
	void CameraZoom(const FInputActionValue& value);
	
	UFUNCTION()
	void CameraFocus(const FInputActionValue& value);

#pragma endregion
#pragma region Spawn Actor
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actor")
	TSubclassOf<AEDCursorActor> CursorActorClass;
	UPROPERTY()
	TObjectPtr<AEDCursorActor> CursorActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Actor")
	TSubclassOf<AEDCameraActor> CameraActorClass;
	UPROPERTY()
	TObjectPtr<AEDCameraActor> CameraActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UEDLootInteractionComponent> LootInteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UEDCraftingInteractionComponent> CraftingInteractionComponent;
#pragma endregion
private:
#pragma region UI 
	// 루팅 인벤토리 패널 열기/닫기 입력 처리
	void HandleToggleLootInventory();

	// 아이템 제작 패널 열기/닫기 입력 처리
	void HandleToggleCraftPanel();

	// ESC 입력 시 패널 닫기 또는 Pause 메뉴 열기 처리
	void HandleUIBack();

	// 제작 입력 시 현재 선택된 레시피 제작을 시도
	void HandleCraftItem();

	// 애플리케이션 복귀 시 현재 열린 UI 상태에 맞게 입력 모드와 포커스 복구를 요청
	void HandleApplicationReactivated();

	// 사망 / 부활 / 매치 결과 UI가 열려 있을 때 HUD 입력을 막음
	bool ShouldBlockHUDInput() const;
	
#pragma endregion 김동주

#pragma region Delegate
	FOnOtherInput OnCameraScroll;
	FOnOtherInput OnCameraFocus;
	FOnCraftInputTriggered OnCraftInputTriggered;

#pragma endregion

	FGenericTeamId CachedTeamId;
	
};
