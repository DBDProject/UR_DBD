// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class UR_DBDproject : ModuleRules
{
    public UR_DBDproject(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[]
        {
            "UR_DBDproject",
        });

        PublicDependencyModuleNames.AddRange(new string[]
    {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayTags",
            "GameplayTasks",
            "GameplayAbilities",
            "NavigationSystem",
            "AIModule",
            "Niagara",
            "UMG",
            "CinematicCamera",
            "LevelSequence",
            "MovieScene",
            "MovieSceneTracks"
    });

        // Add additional required modules for platform-specific operations
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicDefinitions.Add("NOMINMAX");
            PublicDefinitions.Add("WIN32_LEAN_AND_MEAN");
        }
        PublicDependencyModuleNames.AddRange(new string[] { "DBDUI", "DBDNetCore" });
    }
}
