using UnrealBuildTool;
using System.IO;

// ReSharper disable once InconsistentNaming
public class GameCore : ModuleRules
{
	// ReSharper disable once InconsistentNaming
	public GameCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine",
			"GameUtils",
			"GameplayAbilities", "GameplayTags", "GameplayTasks"
		]);

		PrivateDependencyModuleNames.AddRange([]);
	}
}
