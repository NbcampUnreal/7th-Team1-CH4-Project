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
	
	UDataTable* SkillDT = SkillSettings->SkillMulDataTable.LoadSynchronous();
	
	if (IsValid(SkillDT))
	{
		SkillDT->ForeachRow<FEDSkillMulTypeRow>(TEXT("EDSkillDataSubsystem Init"), 
			[this](const FName& Key, const FEDSkillMulTypeRow& Value)
			{
				SkillMulMap.Add(Value.SkillTag, Value.SkillMultiplier);
			});
	}
}

const UEDSkillDataSubsystem* UEDSkillDataSubsystem::Get(UWorld* World)
{
	return World->GetGameInstance()->GetSubsystem<UEDSkillDataSubsystem>();
}

const FSkillMulStatus* UEDSkillDataSubsystem::GetSkillData(FGameplayTag SkillTag) const
{
	return SkillMulMap.Find(SkillTag);
}
