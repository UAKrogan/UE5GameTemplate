using UnrealBuildTool;
using System.IO;

// ReSharper disable once InconsistentNaming
public class GameFramework : ModuleRules
{
	// ReSharper disable once InconsistentNaming
	public GameFramework(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule",
			"GameUtils"
		]);

		PrivateDependencyModuleNames.AddRange([
			"Slate", "SlateCore"
		]);
	}
}