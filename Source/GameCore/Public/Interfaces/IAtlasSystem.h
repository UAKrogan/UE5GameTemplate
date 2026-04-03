#pragma once

#include "CoreMinimal.h"

class UAtlasGameInstanceSubsystem;

/*
 * Base interface for all Atlas systems.
 *
 * Systems are:
 * - Owned by UAtlasGameInstanceSubsystem
 * - Initialized once per game session
 * - Pure logic containers (no direct Actor ownership)
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
	 * Should be stable across the lifetime of the application.
	 */
	virtual FName GetSystemName() const = 0;
};