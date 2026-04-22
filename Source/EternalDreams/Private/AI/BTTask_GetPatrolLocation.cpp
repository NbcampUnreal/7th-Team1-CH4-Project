// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_GetPatrolLocation.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Monster/EDMonsterBase.h"
#include "Environment/EDRestrictedArea.h"
#include "EngineUtils.h"

UBTTask_GetPatrolLocation::UBTTask_GetPatrolLocation()
{
	NodeName = TEXT("Get Patrol Location");
}

EBTNodeResult::Type UBTTask_GetPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController) == false)
		return EBTNodeResult::Failed;
	
	AEDMonsterBase* Monster = Cast<AEDMonsterBase>(AIController->GetPawn());
	if (IsValid(Monster) == false)
		return EBTNodeResult::Failed;
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (IsValid(BB) == false)
		return EBTNodeResult::Failed;
	
	FVector PatrolOrigin = FindSafePatrolOrigin(Monster->GetWorld(), Monster->GetOriginLocation());
	FVector2D RandPoint = FMath::RandPointInCircle(PatrolRadius);
	FVector PatrolLocation = PatrolOrigin + FVector(RandPoint.X, RandPoint.Y, 0.f);
	BB->SetValueAsVector(TEXT("PatrolLocation"), PatrolLocation);
	return EBTNodeResult::Succeeded;
}

FVector UBTTask_GetPatrolLocation::FindSafePatrolOrigin(UWorld* World, const FVector& CurrentOrigin) const
{
	if (IsValid(World) == false)
		return CurrentOrigin;
	
	bool bOriginRestricted = false;
	TArray<AEDRestrictedArea*> SafeZones;
	
	for (TActorIterator<AEDRestrictedArea> It(World); It; ++It)
	{
		AEDRestrictedArea* Zone = *It;
		if (IsValid(Zone) == false)
			continue;
		
		UStaticMeshComponent* Mesh = Zone->FindComponentByClass<UStaticMeshComponent>();
		if (IsValid(Mesh) == false)
			continue;
		FBox Bounds = Mesh->Bounds.GetBox();
		bool bZoneActive = Mesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
		
		if (bZoneActive == false)
		{
			SafeZones.Add(Zone);
			continue;
		}
		
		if (Bounds.IsInsideOrOn(CurrentOrigin))
			bOriginRestricted = true;
	}
	
	if (bOriginRestricted == false)
		return CurrentOrigin;
	
	if (SafeZones.IsEmpty())
		return CurrentOrigin;
	
	AEDRestrictedArea* ClosestZone = nullptr;
	float MinDist = FLT_MAX;
	for (AEDRestrictedArea*Zone : SafeZones)
	{
		float Dist = FVector::Dist(CurrentOrigin, Zone->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist = Dist;
			ClosestZone = Zone;
		}
	}
	return IsValid(ClosestZone) ? ClosestZone->GetActorLocation() : CurrentOrigin;
}
