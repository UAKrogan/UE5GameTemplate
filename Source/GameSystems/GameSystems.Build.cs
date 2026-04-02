using UnrealBuildTool;
using System.IO;

// ReSharper disable once InconsistentNaming
public class GameSystems : ModuleRules
{
	// ReSharper disable once InconsistentNaming
	public GameSystems(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine",
			"GameUtils"
		]);

		PrivateDependencyModuleNames.AddRange([]);
	}
}