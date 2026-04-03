#pragma once

#include "Systems/Load/IAtlasLoadSystem.h"

class UAtlasGameInstanceSubsystem;

/*
 * Concrete implementation of Load system.
 *
 * Handles:
 * - Restoring saved data
 * - Reconstructing runtime state
 */
class GAMECORE_API FAtlasLoadSystem : public IAtlasLoadSystem
{
public:
	virtual void Initialize(UAtlasGameInstanceSubsystem* Subsystem) override;
	virtual void Shutdown() override;

	virtual FName GetSystemName() const override
	{
		return "LoadSystem";
	}

	virtual void LoadGame() override;

private:
	/*
	 * Reference to owning subsystem.
	 */
	UAtlasGameInstanceSubsystem* OwningSubsystem = nullptr;
};
