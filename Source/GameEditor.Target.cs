using UnrealBuildTool;

// ReSharper disable once InconsistentNaming
public class GameEditorTarget : TargetRules
{
	// ReSharper disable once InconsistentNaming
	public GameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Game");
	}
}