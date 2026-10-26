using UnrealBuildTool;
public class GunyuTacticsTarget : TargetRules
{
    public GunyuTacticsTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("GunyuTactics");
    }
}
