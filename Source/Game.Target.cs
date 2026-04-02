using UnrealBuildTool;

// ReSharper disable once InconsistentNaming
public class GameTarget : TargetRules
{
	// ReSharper disable once InconsistentNaming
	public GameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Game");
	}
}