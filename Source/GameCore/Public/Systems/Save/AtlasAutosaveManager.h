#pragma once

#include "CoreMinimal.h"

/*
 * Thread-safe manager for Atlas autosave ring-buffer slots.
 *
 * Slot layout:
 * - Autosave_0
 * - Autosave_1
 * - Autosave_2
 * - Autosave_3
 * - Autosave_4
 *
 * The last reserved index is stored in Saved/SaveGames/AutosaveIndex.sav.
 */
class GAMECORE_API FAtlasAutosaveManager
{
public:
	static constexpr int32 AutosaveSlotCount = 5;

	/*
	 * Reserves the next autosave slot and persists the reserved index.
	 *
	 * This must happen before the async save starts so concurrent autosave
	 * requests do not target the same slot.
	 */
	static bool ReserveNextAutosaveSlot(FString& OutSlotName, int32& OutSlotIndex);

	static FString BuildAutosaveSlotName(int32 SlotIndex);
	static FString GetIndexSlotName();

private:
	static bool LoadReservedIndex(int32& OutIndex);
	static bool StoreReservedIndex(int32 Index);
	static bool IsValidAutosaveIndex(int32 Index);
};
