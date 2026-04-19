#pragma once

#include "CoreMinimal.h"

/*
 * Binary file storage for Atlas save slots.
 *
 * Responsibilities:
 * - Save raw binary data to Saved/SaveGames/
 * - Load raw binary data from Saved/SaveGames/
 * - Keep disk I/O separate from snapshot serialization
 */
class GAMECORE_API FAtlasFileStorage
{
public:
	static constexpr const TCHAR* SaveExtension = TEXT(".sav");
	static constexpr const TCHAR* TempExtension = TEXT(".tmp");

	static bool SaveToSlot(const FString& SlotName, const TArray<uint8>& Data);
	static bool LoadFromSlot(const FString& SlotName, TArray<uint8>& OutData);

	static bool DoesSlotExist(const FString& SlotName);
	static bool DeleteSlot(const FString& SlotName);

	static FString GetSaveDirectory();
	static FString GetSlotFilePath(const FString& SlotName);
	static FString GetTempSlotFilePath(const FString& SlotName);

private:
	static bool IsValidSlotName(const FString& SlotName);
	static FString SanitizeSlotName(const FString& SlotName);
	static bool EnsureSaveDirectoryExists();
};
