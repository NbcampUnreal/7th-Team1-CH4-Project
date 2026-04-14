#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EDTestLootContainer.generated.h"

class UStaticMeshComponent;
class UEDInventoryComponent;

UCLASS()
class ETERNALDREAMS_API AEDTestLootContainer : public AActor
{
	GENERATED_BODY()

public:
	AEDTestLootContainer();

	// 상자 인벤토리 컴포넌트 반환
	UFUNCTION(BlueprintCallable, Category = "Loot")
	UEDInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

protected:
	// 상자 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	// 상자 인벤토리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UEDInventoryComponent> InventoryComponent;
};
