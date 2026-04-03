#pragma once

class UAtlasGameInstanceSubsystem;

/*
 * FAtlasSystemsRegistrar
 *
 * Responsible for registering all systems from GameSystems module.
 *
 * This avoids circular dependencies:
 * - GameCore does NOT know about GameSystems
 * - GameSystems injects its systems into GameCore
 */
class GAMECORE_API FAtlasSystemsRegistry
{
public:
	/*
	 * Registers all systems into the given subsystem.
	 */
	static void Register(UAtlasGameInstanceSubsystem* Subsystem);
};
