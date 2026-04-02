using UnrealBuildTool;
using System.IO;

// ReSharper disable once InconsistentNaming
public class GameUtils : ModuleRules
{
	// ReSharper disable once InconsistentNaming
	public GameUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine"
		]);

		PrivateDependencyModuleNames.AddRange([]);
	}
}