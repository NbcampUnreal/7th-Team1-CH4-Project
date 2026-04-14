#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDTestLootContainer.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UEDInventoryComponent;

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
	virtual void BeginPlay() override;

	// 플레이어가 루팅 가능 범위에 들어왔을 때 호출
	UFUNCTION()
	void HandleInteractionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 플레이어가 루팅 가능 범위에서 벗어났을 때 호출
	UFUNCTION()
	void HandleInteractionEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	// 시각용 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	// 상호작용 범위 감지 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionBox;

	// 루팅 대상 인벤토리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;
};
