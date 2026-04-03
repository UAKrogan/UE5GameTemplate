#pragma once

#include "Interfaces/IAtlasSystem.h"

/*
 * Interface for Load system.
 *
 * Responsible for:
 * - Loading game state
 * - Restoring runtime data
 */
class GAMECORE_API IAtlasLoadSystem : public IAtlasSystem
{
public:
	static FName InterfaceName()
	{
		return "IAtlasLoadSystem";
	}

	/*
	 * Triggers load process.
	 */
	virtual void LoadGame() = 0;
};
