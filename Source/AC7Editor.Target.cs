// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class AC7EditorTarget : TargetRules
{
	public AC7EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		//ExtraModuleNames.Add("AC7");
		ExtraModuleNames.AddRange(new string[] { "AC7", "CustomEditor" });
	}
}
