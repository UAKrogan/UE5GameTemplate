#include "Subsystems/AtlasGameInstanceSubsystem.h"

#include "Interfaces/IAtlasSystem.h"
#include "Logging/AtlasLogMacros.h"
#include "Systems/AtlasSystemsRegistry.h"

/*
 * Called when GameInstance is initialized.
 *
 * Initializes all Atlas systems.
 */
void UAtlasGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ATLAS_LOG_CORE(Log, "AtlasGameInstanceSubsystem Initialize");

	RegisterSystems();
	InitializeSystems();
}

/*
 * Called during shutdown.
 *
 * Ensures all systems are properly cleaned up.
 */
void UAtlasGameInstanceSubsystem::Deinitialize()
{
	ATLAS_LOG_CORE(Log, "AtlasGameInstanceSubsystem Deinitialize");

	ShutdownSystems();

	Super::Deinitialize();
}

/*
 * Delegates system registration to GameSystems module.
 */
void UAtlasGameInstanceSubsystem::RegisterSystems()
{
	ATLAS_LOG_CORE(Log, "Registering systems");

	FAtlasSystemsRegistry::Register(this);
}

/*
 * Calls Initialize() on all registered systems.
 */
void UAtlasGameInstanceSubsystem::InitializeSystems()
{
	for (auto& Pair : Systems)
	{
		ATLAS_LOG_CORE(Log, "Initializing system: %s", *Pair.Key.ToString());

		Pair.Value->Initialize(this);
	}
}

/*
 * Calls Shutdown() on all systems and clears registry.
 */
void UAtlasGameInstanceSubsystem::ShutdownSystems()
{
	for (auto& Pair : Systems)
	{
		ATLAS_LOG_CORE(Log, "Shutting down system: %s", *Pair.Key.ToString());

		Pair.Value->Shutdown();
	}

	Systems.Empty();
}

/*
 * Registers a system instance into internal storage.
 */
void UAtlasGameInstanceSubsystem::RegisterSystem(TSharedPtr<IAtlasSystem> System)
{
	if (!System.IsValid())
	{
		return;
	}

	Systems.Add(System->GetSystemName(), System);
}
