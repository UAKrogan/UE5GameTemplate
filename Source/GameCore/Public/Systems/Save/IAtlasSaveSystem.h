#pragma once

#include "Interfaces/IAtlasSystem.h"

/*
 * Interface for Save system.
 *
 * Responsible for:
 * - Saving game state
 * - Serializing runtime data
 */
class GAMECORE_API IAtlasSaveSystem : public IAtlasSystem
{
public:
	static FName InterfaceName()
	{
		return "IAtlasSaveSystem";
	}

	/*
	 * Triggers save process.
	 */
	virtual void SaveGame() = 0;
};
