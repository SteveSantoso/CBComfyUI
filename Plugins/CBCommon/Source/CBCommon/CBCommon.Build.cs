// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class CBCommon : ModuleRules
{
	public CBCommon(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		
		string ProjectConfigDir = "$(ProjectDir)/Config/ProjectJsonConfig";
		RuntimeDependencies.Add(String.Format("{0}/*", ProjectConfigDir));
		
		string ActorJsonConfigDir = "$(ProjectDir)/Config/ActorJsonConfig";
		RuntimeDependencies.Add(String.Format("{0}/*", ActorJsonConfigDir));
		
		string CBExtrasDir = Path.Combine(PluginDirectory, "Extras");
		foreach (string DataFile in Directory.EnumerateFiles(CBExtrasDir, "*", SearchOption.AllDirectories))
		{
			RuntimeDependencies.Add(DataFile);
		}
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Json",
                "RHI",
                "RenderCore",
                "Projects",
                "DeveloperSettings",
                "JsonUtilities",
                "AudioCapture",
                "AudioMixer",
            }
			);
		
		
		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"DesktopPlatform",
					"MessageLog",
					"UnrealEd",
				}
			);
		}
	}
}
