#pragma once

#include "Interfaces/IAtlasSystem.h"
#include "SaveGame/AtlasSaveGameTypes.h"

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
	 * Triggers load process using the default slot.
	 */
	virtual void LoadGame() = 0;

	/*
	 * Loads a snapshot from a specific slot.
	 *
	 * @return true if the slot existed and was loaded successfully
	 */
	virtual bool LoadGameFromSlot(const FString& SlotName, int32 UserIndex = 0) = 0;

	/*
	 * Returns the default save slot used by LoadGame().
	 */
	virtual FString GetDefaultSlotName() const = 0;

	/*
	 * Checks whether a save exists for the requested slot.
	 */
	virtual bool DoesSaveExist(const FString& SlotName, int32 UserIndex = 0) const = 0;

	/*
	 * Clears the cached loaded snapshot.
	 */
	virtual void ClearLoadedSnapshot() = 0;

	/*
	 * Returns the last successfully loaded metadata.
	 */
	virtual bool GetLoadedMetadata(FAtlasSaveGameMetadata& OutMetadata) const = 0;

	/*
	 * Returns the last successfully loaded snapshot.
	 */
	virtual bool GetLoadedSnapshot(FAtlasSaveGameSnapshot& OutSnapshot) const = 0;

	virtual bool TryGetIntValue(FName Key, int32& OutValue) const = 0;
	virtual bool TryGetFloatValue(FName Key, float& OutValue) const = 0;
	virtual bool TryGetStringValue(FName Key, FString& OutValue) const = 0;
	virtual bool TryGetVectorValue(FName Key, FVector& OutValue) const = 0;
	virtual bool TryGetRotatorValue(FName Key, FRotator& OutValue) const = 0;
	virtual bool TryGetTransformValue(FName Key, FTransform& OutValue) const = 0;
};
