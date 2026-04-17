#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDLootTargetComponent.generated.h"

class UBoxComponent;

/**
 * 루팅 가능한 액터를 플레이어의 현재 루팅 대상으로 등록/해제하는 컴포넌트
 * 일반 Actor 블루프린트에 InventoryComponent와 함께 붙이면, 내부적으로 박스 충돌을 만들어 루팅 범위를 구성
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UEDLootTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEDLootTargetComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 플레이어가 루팅 범위에 들어왔을 때 현재 루팅 대상으로 등록
	UFUNCTION()
	void HandleInteractionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 플레이어가 루팅 범위에서 벗어났을 때 현재 루팅 대상을 해제
	UFUNCTION()
	void HandleInteractionEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	void CreateInteractionBox();
	void BindInteractionCollision();
	void UnbindInteractionCollision();

protected:
	// 루팅 범위 박스의 상대 위치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FVector TriggerRelativeLocation = FVector::ZeroVector;

	// 루팅 범위 박스의 크기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FVector TriggerBoxExtent = FVector(120.0f, 120.0f, 120.0f);

	// 내부 박스 콜리전 컴포넌트 이름
	UPROPERTY(Transient)
	TObjectPtr<UBoxComponent> InteractionBox = nullptr;
};
