// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SkylakeSharedClient : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string SkylakeSharedLibraryName = "SkylakeShared_Client.lib"; //Name of library linked with SkylakeLibStandalone

    private string SkylakeSharedLibraryPathBase
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../../server/SkylakeSharedLib/")); } // path to where the library binary is
    }

    private string SkylakeSharedLibraryIncludePath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../../server/source/Shared/Public/")); } // project's include directory
    }

    private string GetSkylakeSharedLibraryFullName(ReadOnlyTargetRules Target)
    {
        var SkylakeSharedLibraryPath = $"{SkylakeSharedLibraryPathBase}";
        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            SkylakeSharedLibraryPath += "Debug/";
        }
        else
        {
            SkylakeSharedLibraryPath += "Release/";
        }
        SkylakeSharedLibraryPath += SkylakeSharedLibraryName;

        return SkylakeSharedLibraryPath;
    }

    public SkylakeSharedClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicAdditionalLibraries.Add(GetSkylakeSharedLibraryFullName(Target));
        PublicIncludePaths.Add(SkylakeSharedLibraryIncludePath);

        PublicDefinitions.Add("SKL_CLIENT=1");
        PublicDefinitions.Add("SKL_L1_CACHE_LINE_64=1");

        if(Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            PublicDefinitions.Add("SKL_BUILD_SHIPPING=1");
            PublicDefinitions.Add("SKLL_LOG_LEVEL=5"); //SKLL_LOG_LEVEL_MUTE
        }
        else
        {
            PublicDefinitions.Add("SKLL_LOG_LEVEL=1"); //SKLL_LOG_LEVEL_DEBUG
        }
    }
}
