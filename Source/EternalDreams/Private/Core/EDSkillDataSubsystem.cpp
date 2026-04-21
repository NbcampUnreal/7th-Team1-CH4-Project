// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/EDSkillDataSubsystem.h"

#include "Data/EDSkillDeveloperSettings.h"
#include "Data/EDSkillMulTypeRow.h"

void UEDSkillDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UEDSkillDeveloperSettings* SkillSettings = GetDefault<UEDSkillDeveloperSettings>();
	if(!IsValid(SkillSettings))
	{
		return;	
	}
	
	SkillDT = SkillSettings->SkillMulDataTable.LoadSynchronous();
	EnemyMat=SkillSettings->EnemyOverlayMat.LoadSynchronous();
	TeamMat=SkillSettings->TeamOverlayMat.LoadSynchronous();
	
	if (EnemyMat&&TeamMat)
	{
		UE_LOG(LogTemp,Warning,TEXT("EDSkillDataSubsystem : LoadComplete"));
	}
	
	
	if (IsValid(SkillDT))
	{
		SkillDT->ForeachRow<FEDSkillMulTypeRow>(TEXT("EDSkillDataSubsystem Init"), 
			[this](const FName& Key, const FEDSkillMulTypeRow& Value)
			{
				SkillMulMap.Add(Value.SkillTag, Value.SkillMultiplier);
			});
	}
}

UEDSkillDataSubsystem* UEDSkillDataSubsystem::Get(UWorld* World)
{
	return World->GetGameInstance()->GetSubsystem<UEDSkillDataSubsystem>();
}

const FSkillMulStatus* UEDSkillDataSubsystem::GetSkillData(FGameplayTag SkillTag) const
{
	return SkillMulMap.Find(SkillTag);
}

UMaterial* UEDSkillDataSubsystem::GetEnemyMat()
{
	if (IsValid(EnemyMat))
	{
		return EnemyMat;
	}
	const UEDSkillDeveloperSettings* SkillSettings = GetDefault<UEDSkillDeveloperSettings>();
	if(!IsValid(SkillSettings))
	{
		UE_LOG(LogTemp,Warning,TEXT("SkillSettings Nullptr"));
		return nullptr;	
	}
	if (SkillSettings->EnemyOverlayMat.IsValid())
	{
		EnemyMat=SkillSettings->EnemyOverlayMat.LoadSynchronous();
		return EnemyMat;
	}
	UE_LOG(LogTemp,Warning,TEXT("EDSkillDataSubsystem : EnemyMatNotLoaded"));
	return nullptr;
}

UMaterial* UEDSkillDataSubsystem::GetTeamMat()
{
	if (IsValid(TeamMat))
	{
		return TeamMat;
	}
	const UEDSkillDeveloperSettings* SkillSettings = GetDefault<UEDSkillDeveloperSettings>();
	if(!IsValid(SkillSettings))
	{
		UE_LOG(LogTemp,Warning,TEXT("SkillSettings nullptr"));
		return nullptr;	
	}
	if (SkillSettings->TeamOverlayMat.IsValid())
	{
		TeamMat=SkillSettings->TeamOverlayMat.LoadSynchronous();
		return TeamMat;
	}
	UE_LOG(LogTemp,Warning,TEXT("EDSkillDataSubsystem : TeamMatNotLoaded"));
	return nullptr;
}
