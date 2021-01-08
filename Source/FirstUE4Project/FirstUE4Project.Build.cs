// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FirstUE4Project : ModuleRules
{
	public FirstUE4Project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
		
		PrivateDependencyModuleNames.AddRange(new [] { "NinjaCharacter" });
	}
}
