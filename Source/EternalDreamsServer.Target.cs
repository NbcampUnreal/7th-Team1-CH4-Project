// Copyright Eternal Dreams Team. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EternalDreamsServerTarget : TargetRules
{
	public EternalDreamsServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("EternalDreams");
	}
}
