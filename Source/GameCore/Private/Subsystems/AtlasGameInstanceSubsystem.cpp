#include "Subsystems/AtlasGameInstanceSubsystem.h"

#include "Interfaces/IAtlasSystem.h"
#include "Logging/AtlasLogMacros.h"

void UAtlasGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ATLAS_LOG_CORE(Log, "AtlasGameInstanceSubsystem Initialize");

	RegisterSystems();
	InitializeSystems();
}

void UAtlasGameInstanceSubsystem::Deinitialize()
{
	ATLAS_LOG_CORE(Log, "AtlasGameInstanceSubsystem Deinitialize");

	ShutdownSystems();

	Super::Deinitialize();
}

void UAtlasGameInstanceSubsystem::RegisterSystems()
{
	ATLAS_LOG_CORE(Log, "Registering systems");

	/*
	 * Register systems here.
	 *
	 * Example (to be implemented later):
	 *
	 * Systems.Add("Save", MakeShared<FAtlasSaveSystem>());
	 *
	 * Rules:
	 * - Systems are created ONLY here
	 * - No dynamic creation elsewhere
	 */
}

void UAtlasGameInstanceSubsystem::InitializeSystems()
{
	for (auto& Pair : Systems)
	{
		ATLAS_LOG_CORE(Log, "Initializing system: %s", *Pair.Key.ToString());
		Pair.Value->Initialize(this);
	}
}

void UAtlasGameInstanceSubsystem::ShutdownSystems()
{
	for (auto& Pair : Systems)
	{
		ATLAS_LOG_CORE(Log, "Shutting down system: %s", *Pair.Key.ToString());
		Pair.Value->Shutdown();
	}

	Systems.Empty();
}