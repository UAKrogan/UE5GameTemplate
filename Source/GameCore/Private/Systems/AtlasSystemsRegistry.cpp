#include "Systems/AtlasSystemsRegistry.h"

#include "Subsystems/AtlasGameInstanceSubsystem.h"
#include "Systems/Save/AtlasSaveSystem.h"
#include "Systems/Load/AtlasLoadSystem.h"

/*
 * Registers all GameSystems systems into the subsystem.
 */
void FAtlasSystemsRegistry::Register(UAtlasGameInstanceSubsystem* Subsystem)
{
	if (!Subsystem)
	{
		return;
	}

	// Register Save system
	TSharedPtr<IAtlasSystem> SaveSystem = MakeShared<FAtlasSaveSystem>();
	Subsystem->RegisterSystem(SaveSystem);

	// Register Load system
	TSharedPtr<IAtlasSystem> LoadSystem = MakeShared<FAtlasLoadSystem>();
	Subsystem->RegisterSystem(LoadSystem);
}
