#include "Systems/Save/AtlasSaveSystem.h"

#include "Logging/AtlasLogMacros.h"

#include "Engine/World.h"

/*
 * Called during system initialization.
 */
void FAtlasSaveSystem::Initialize(UAtlasGameInstanceSubsystem* Subsystem)
{
	OwningSubsystem = Subsystem;

	ATLAS_LOG_CORE(Log, "SaveSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasSaveSystem::Shutdown()
{
	ATLAS_LOG_CORE(Log, "SaveSystem shutdown");
}

/*
 * Entry point for saving game state.
 */
void FAtlasSaveSystem::SaveGame()
{
	ATLAS_LOG_CORE(Log, "SaveGame called");
}
