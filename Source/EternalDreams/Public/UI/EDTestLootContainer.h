#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDTestLootContainer.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UEDInventoryComponent;
class UEDLootTargetComponent;

/**
 * 구형 테스트 루팅 액터 호환용 래퍼
 * 새 구조에서는 일반 액터에 InventoryComponent와 UEDLootTargetComponent를 붙여서 사용
 * 기존 블루프린트 자산이 바로 깨지지 않도록 최소 구성만 유지
 */
UCLASS()
class ETERNALDREAMS_API AEDTestLootContainer : public AActor
{
	GENERATED_BODY()

public:
	AEDTestLootContainer();

	// 외부에서 직접 접근할 수 있도록 인벤토리 컴포넌트를 반환
	UFUNCTION(BlueprintCallable, Category = "Loot")
	UEDInventoryComponent* GetInventoryComponent() const;

protected:
	// 시각용 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 상호작용 범위 감지 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionBox;

	// 루팅 대상 인벤토리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;

	// 루팅 대상 등록/해제를 담당하는 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEDLootTargetComponent> LootTargetComponent;
};
