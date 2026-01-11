// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class SkyEngineUE : ModuleRules
{
	public SkyEngineUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		AddSkyEngineLibrary(Target);
		AddSkyThirdParty(Target);
		
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
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "Json",
                "JsonUtilities"
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);

		PrivateDefinitions.Add("SKY_EDITOR=1");
	}

    public bool AddSkyEngineLibrary(ReadOnlyTargetRules Target)
    {

		string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty")); 
        string LibPath = Path.Combine(ThirdPartyPath, "SkyEngineRuntime");
        string IncludePath = Path.Combine(LibPath, "include");
        string LibsPath = Path.Combine(LibPath, "lib");

        PublicIncludePaths.Add(IncludePath);

        PublicAdditionalLibraries.Add(Path.Combine(LibsPath, "Core.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibsPath, "Framework.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibsPath, "Animation.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibsPath, "RenderAdaptor.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibsPath, "RenderCore.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibsPath, "RHI.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(LibsPath, "ShaderCompiler.Static.lib"));

        return true;
    }

    public bool AddSkyThirdParty(ReadOnlyTargetRules Target)
	{
		string[] libs = new string[]
		{
			"boost",
			"rapidjson",
			"crc32c",
			"taskflow",
			"sfmt",
			"animation",
			"shader"
		};

        string ThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty"));
        foreach (string lib in libs)
        {
            string LibPath = Path.Combine(ThirdPartyPath, lib);
            string IncludePath = Path.Combine(LibPath, "include");
            PublicIncludePaths.Add(IncludePath);
        }

#if DEVELOPMENT
		string Prefix = "Release";
#else
		string Prefix = "Release";
#endif

		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "sfmt",   "lib", Prefix, "SFMT.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "crc32c", "lib", Prefix, "crc32c.lib"));

		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_atomic.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_chrono.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_container.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_date_time.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_graph.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_random.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_serialization.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_thread.lib"));
		PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "boost", "lib", Prefix, "libboost_wserialization.lib"));

        return true;
	}
}
