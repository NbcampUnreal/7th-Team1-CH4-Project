// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EternalDreams : ModuleRules
{
	public EternalDreams(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"NavigationSystem",
			"AIModule",
			"UMG",
			"CommonUI",
			"CommonInput",
			"GameplayAbilities", 
			"DeveloperSettings",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayTags", "GameplayTasks", "MoviePlayer", "Niagara",
		});
		
		PublicIncludePaths.AddRange(new string[] { "EternalDreams" });

		// Slate UI — 로딩 화면 위젯에서 사용
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
