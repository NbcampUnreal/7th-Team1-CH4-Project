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
		});

		PrivateDependencyModuleNames.AddRange(new string[] 
		{
			"GameplayTags", "GameplayTasks", "Slate", "SlateCore",
		});
		
		PublicIncludePaths.AddRange(new string[] { "EternalDreams" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
