#pragma once

#include "Interfaces/IAtlasSystem.h"
#include "SaveGame/AtlasSaveGameTypes.h"

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
	 * Triggers save process using the default slot.
	 */
	virtual void SaveGame() = 0;

	/*
	 * Queues an async save request for a specific slot.
	 *
	 * @return true if the request was accepted
	 */
	virtual bool RequestSave(const FString& SlotName) = 0;

	/*
	 * Queues an async autosave request.
	 *
	 * @return true if the request was accepted
	 */
	virtual bool RequestAutosave() = 0;

	/*
	 * Saves the current snapshot to a specific slot.
	 *
	 * @return true if the save request was accepted
	 */
	virtual bool SaveGameToSlot(const FString& SlotName, int32 UserIndex = 0) = 0;

	/*
	 * Returns the default save slot used by SaveGame().
	 */
	virtual FString GetDefaultSlotName() const = 0;

	/*
	 * Checks whether a save exists for the requested slot.
	 */
	virtual bool DoesSaveExist(const FString& SlotName, int32 UserIndex = 0) const = 0;

	/*
	 * Clears the pending snapshot while keeping the system alive.
	 */
	virtual void ClearPendingSnapshot() = 0;

	virtual void SetIntValue(FName Key, int32 Value) = 0;
	virtual void SetFloatValue(FName Key, float Value) = 0;
	virtual void SetStringValue(FName Key, const FString& Value) = 0;
	virtual void SetVectorValue(FName Key, const FVector& Value) = 0;
	virtual void SetRotatorValue(FName Key, const FRotator& Value) = 0;
	virtual void SetTransformValue(FName Key, const FTransform& Value) = 0;

	/*
	 * Removes a value from any supported snapshot store.
	 *
	 * @return true if a value existed and was removed
	 */
	virtual bool RemoveValue(FName Key) = 0;

	/*
	 * Copies the current pending snapshot.
	 *
	 * @return true if the snapshot contains any data or metadata
	 */
	virtual bool GetPendingSnapshot(FAtlasSaveGameSnapshot& OutSnapshot) const = 0;
};
