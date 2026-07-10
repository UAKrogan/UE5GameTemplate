using UnrealBuildTool;
using System.IO;

// ReSharper disable once InconsistentNaming
public class GameActors : ModuleRules
{
	// ReSharper disable once InconsistentNaming
	public GameActors(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

		PublicDependencyModuleNames.AddRange([
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule",
			"GameplayAbilities", "GameplayTags", "GameplayTasks",
			"ModularGameplay", "GameFeatures",
			"GameUtils"
		]);

		PrivateDependencyModuleNames.AddRange([]);
	}
}