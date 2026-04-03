#pragma once

#include "CoreMinimal.h"

/*
 * Interface for savable actors.
 *
 * Actors implementing this interface are responsible
 * for serializing their own state.
 *
 * Phase 1:
 * - No real serialization yet
 * - Only logging to validate execution flow
 */
class GAMECORE_API IAtlasSavable
{
public:
	virtual ~IAtlasSavable() = default;

	/*
	 * Called by SaveSystem.
	 */
	virtual void Save(FArchive& Ar) = 0;

	/*
	 * Called by LoadSystem.
	 */
	virtual void Load(FArchive& Ar) = 0;
};
