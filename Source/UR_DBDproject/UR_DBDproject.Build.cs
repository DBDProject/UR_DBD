// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class UR_DBDproject : ModuleRules
{
    public UR_DBDproject(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        string ProtobufPath = Path.Combine(ModuleDirectory, "../../ThirdParty/Protobuf");

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
            "UMG"
    });

        PublicIncludePaths.Add(Path.Combine(ProtobufPath, "include"));
        PublicAdditionalLibraries.Add(Path.Combine(ProtobufPath, "lib", "protobuf.lib"));
        RuntimeDependencies.Add(Path.Combine(ProtobufPath, "bin", "protobuf.dll"));
    }
}
