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
        PublicDefinitions.Add("NOMINMAX");
        PublicDependencyModuleNames.AddRange(new string[] { "DBDUI", "DBDNetCore" });
    }
}
