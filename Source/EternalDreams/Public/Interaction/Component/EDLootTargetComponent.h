#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EDLootTargetComponent.generated.h"

class UPrimitiveComponent;

/**
 * 루팅 가능한 액터를 플레이어의 현재 루팅 대상으로 등록/해제하는 컴포넌트
 * 블루프린트에서 InventoryComponent와 충돌 컴포넌트를 함께 붙이면
 * 별도의 전용 액터 클래스 없이도 루팅 대상 액터를 만들 수 있음
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ETERNALDREAMS_API UEDLootTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEDLootTargetComponent();

	// 루팅 대상 진입/이탈을 감지할 충돌 컴포넌트를 지정
	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetInteractionCollision(UPrimitiveComponent* InInteractionCollision);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 플레이어가 루팅 가능 범위 안으로 들어왔을 때 현재 루팅 대상으로 등록
	UFUNCTION()
	void HandleInteractionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 플레이어가 루팅 가능 범위를 벗어났을 때 현재 루팅 대상을 해제
	UFUNCTION()
	void HandleInteractionEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

private:
	void BindInteractionCollision();
	void UnbindInteractionCollision();
	UPrimitiveComponent* ResolveInteractionCollision() const;

protected:
	// 오버랩을 감지할 충돌 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UPrimitiveComponent> InteractionCollision = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	bool bUseOwnerRootPrimitiveWhenEmpty = true;
};
