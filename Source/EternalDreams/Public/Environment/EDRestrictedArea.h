// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "EDRestrictedArea.generated.h"

class UGameplayEffect;
class UBoxComponent;

UCLASS()
class ETERNALDREAMS_API AEDRestrictedArea : public AActor
{
	GENERATED_BODY()

public:
	AEDRestrictedArea();

protected:
	virtual void BeginPlay() override;

protected:
	// 금지구역용 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> AreaMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> LineRestrictMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> LineAreaMesh;
	
	
	// 메시 컬러 옵션
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> DynamicLineMaterial;

	UPROPERTY(EditAnywhere, Category = "Settings|Visual")
	FName ColorParamName = TEXT("LineColor");
	
	// 기본 색상 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Visual")
	FLinearColor DefaultLineColor = FLinearColor::White;

	// 금지구역 활성화 시 색상 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Visual")
	FLinearColor ActiveLineColor = FLinearColor::Red;


	
public:
	/** 이 금지구역이 덮는 구역 번호 (1~4). 레벨에서 인스턴스별로 지정 */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "ED|Zone")
	int32 ZoneID = 0;

	/** 금지구역 활성화 — 콜리전 ON + 이미 안에 있는 플레이어 처리 (서버 전용) */
	void ActivateZone();

	// 에디터에서 할당해 줄 게임플레이 이펙트 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> RestrictedAreaEffectClass;

	UFUNCTION()
	void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnMeshEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
