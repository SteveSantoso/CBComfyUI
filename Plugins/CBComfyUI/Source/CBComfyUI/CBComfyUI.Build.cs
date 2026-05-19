// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CBComfyUI : ModuleRules
{
	public CBComfyUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
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
				"CBCommon",
				"WebSockets",
				"HTTP",
				"Json",
				"ImageWrapper",
				"RenderCore",
				"RHI"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
		
		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[] {
					"UnrealEd",
					"AssetRegistry",
					"KismetCompiler",
					"ToolMenus",
					"EditorStyle"
				}
			);
		}

		// Runtime optimizations
		if (Target.Configuration != UnrealTargetConfiguration.Debug)
		{
			bUseUnity = true;
		}

		// Enable exceptions for better error handling
		bEnableExceptions = true;
	}
}
