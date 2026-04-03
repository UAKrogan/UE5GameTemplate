#include "Systems/Load/AtlasLoadSystem.h"

#include "Logging/AtlasLogMacros.h"

#include "Engine/World.h"

/*
 * Called during system initialization.
 */
void FAtlasLoadSystem::Initialize(UAtlasGameInstanceSubsystem* Subsystem)
{
	OwningSubsystem = Subsystem;

	ATLAS_LOG_CORE(Log, "LoadSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasLoadSystem::Shutdown()
{
	ATLAS_LOG_CORE(Log, "LoadSystem shutdown");
}

/*
 * Entry point for loading game state.
 */
void FAtlasLoadSystem::LoadGame()
{
	ATLAS_LOG_CORE(Log, "LoadGame called");
}
