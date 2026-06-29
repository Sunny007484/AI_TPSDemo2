// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class AI_TPSDemo2 : ModuleRules
{
	public AI_TPSDemo2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 允许以模块根为基准用子目录路径包含头文件，例如 #include "Core/TSGameplayTags.h"
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"GameplayAbilities", "GameplayTags", "GameplayTasks",
			"EnhancedInput", "AIModule", "NavigationSystem",
			"UMG", "Slate", "SlateCore", "Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
