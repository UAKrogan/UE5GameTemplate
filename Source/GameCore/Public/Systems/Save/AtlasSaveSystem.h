#pragma once

#include "Systems/Save/IAtlasSaveSystem.h"

class UAtlasGameInstanceSubsystem;

/*
 * Concrete implementation of Save system.
 *
 * Handles:
 * - Collecting savable data
 * - Serializing game state
 */
class GAMECORE_API FAtlasSaveSystem : public IAtlasSaveSystem
{
public:
	virtual void Initialize(UAtlasGameInstanceSubsystem* Subsystem) override;
	virtual void Shutdown() override;

	virtual FName GetSystemName() const override
	{
		return "SaveSystem";
	}

	virtual bool SupportsInterface(FName InterfaceName) const override
	{
		return InterfaceName == IAtlasSaveSystem::InterfaceName();
	}

	virtual void SaveGame() override;

private:
	/*
	 * Reference to owning subsystem.
	 */
	UAtlasGameInstanceSubsystem* OwningSubsystem = nullptr;
};
