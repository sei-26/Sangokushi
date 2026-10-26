using UnrealBuildTool;
public class GunyuTacticsEditorTarget : TargetRules
{
    public GunyuTacticsEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("GunyuTactics");
    }
}
