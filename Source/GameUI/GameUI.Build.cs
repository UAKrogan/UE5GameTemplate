using UnrealBuildTool;
using System.IO;

// ReSharper disable once InconsistentNaming
public class GameUI : ModuleRules
{
	// ReSharper disable once InconsistentNaming
	public GameUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine", "UMG", "CommonUI", "CommonInput",
			"InputCore", "DeveloperSettings",
			"GameUtils"
		]);

		PrivateDependencyModuleNames.AddRange([
			"Slate", "SlateCore"
		]);
	}
}