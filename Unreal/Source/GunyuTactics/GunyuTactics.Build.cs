using UnrealBuildTool;
using System.IO;
public class GunyuTactics : ModuleRules
{
    public GunyuTactics(ReadOnlyTargetRules Target) : base(Target)
    {
        // Keep translation units isolated; Git adaptive unity must not change name lookup.
        bUseUnity = false;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore" });
        PrivateDependencyModuleNames.AddRange(new[] { "Slate", "SlateCore", "ProceduralMeshComponent" });
        PrivateIncludePaths.Add(Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../NewGame")));
        string RulesLibrary = Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Intermediate/Rules/sangokushi_rules.lib"));
        if (!File.Exists(RulesLibrary))
            throw new BuildException("Build the shared rules first: Unreal/Tools/build.ps1");
        PublicAdditionalLibraries.Add(RulesLibrary);
        // Match the engine's STL ABI; simulation code remains in its own CMake library.
        bEnableExceptions = true;
    }
}
