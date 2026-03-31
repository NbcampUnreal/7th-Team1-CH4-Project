// Fill out your copyright notice in the Description page of Project Settings.


#include "UPEdEngine.h"
#include "ISourceControlModule.h"

void UUPEdEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
	
	const ISourceControlModule& SourceControlModule = ISourceControlModule::Get();
	ISourceControlProvider& SourceControlProvider = SourceControlModule.GetProvider();
	const TArray<FString> Branches {
		"origin/main",
		"origin/release",
		"origin/develop",
		"origin/feature/*",
		"origin/hotfix/*"};
	SourceControlProvider.RegisterStateBranches(Branches, TEXT("Content"));
}
