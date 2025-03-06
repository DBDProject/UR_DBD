// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
public class DBDNetCore : ModuleRules
{
    public DBDNetCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Sockets",
                "Networking"
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );

        // Add Protobuf library from custom path
        // Using absolute path to avoid reference problems
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            string ProjectDir = Path.Combine(Directory.GetParent(ModuleDirectory).ToString(), "../../..");
            string ProtobufPath = Path.Combine(ProjectDir, "ThirdParty", "protobuf");

            PublicSystemIncludePaths.Add(Path.Combine(ProtobufPath, "include"));
            PublicAdditionalLibraries.Add(Path.Combine(ProtobufPath, "lib", "libprotobuf.lib"));

            //Unreal이 자동으로 DLL을 로드할 수 있도록 보장


            if (!Directory.Exists(Path.Combine(ProjectDir, "Binaries/Win64")))
            {
                Directory.CreateDirectory(Path.Combine(ProjectDir, "Binaries/Win64"));
            }
            string DestDLL = Path.Combine(ProjectDir,
                "Binaries/Win64", "libprotobuf.dll");

            if (!File.Exists(DestDLL))
            {
                File.Copy(Path.Combine(ProtobufPath, "bin", "libprotobuf.dll"), DestDLL, true);
            }

            PublicDefinitions.Add("GOOGLE_PROTOBUF_NO_RTTI=1");
            PublicDefinitions.Add("GOOGLE_PROTOBUF_USE_UNALIGNED=0");
        }
    }
}