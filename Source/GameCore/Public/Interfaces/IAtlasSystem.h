#pragma once

#include "CoreMinimal.h"

class UAtlasGameInstanceSubsystem;

/*
 * Base class for all Atlas systems.
 *
 * Systems are:
 * - Owned by UAtlasGameInstanceSubsystem
 * - Initialized once per game session
 * - Pure logic containers (no direct Actor ownership)
 *
 * Design rules:
 * - Systems must not depend on Actors directly
 * - Systems must not own gameplay objects
 * - Systems are controlled by the Subsystem lifecycle
 */
class GAMECORE_API IAtlasSystem
{
public:
	virtual ~IAtlasSystem() = default;

	/*
	 * Called during subsystem initialization.
	 *
	 * @param Subsystem - Owning subsystem (service locator / orchestrator)
	 */
	virtual void Initialize(UAtlasGameInstanceSubsystem* Subsystem) = 0;

	/*
	 * Called during shutdown.
	 */
	virtual void Shutdown() = 0;

	/*
	 * Unique system identifier.
	 *
	 * Must be stable and unique across all systems.
	 */
	virtual FName GetSystemName() const = 0;

	/*
	 * Runtime-checked interface support for safe subsystem lookup.
	 *
	 * @param InterfaceName - Logical interface identifier
	 * @return true if this system implements the requested interface
	 */
	virtual bool SupportsInterface(FName InterfaceName) const = 0;
};
