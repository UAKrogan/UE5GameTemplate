#pragma once

#include "CoreMinimal.h"
#include "SaveGame/AtlasSaveGameTypes.h"

/*
 * JSON manifest of available save slots so the UI can list saves without
 * probing/deserializing every slot file.
 *
 * Stored at Saved/SaveGames/SaveManifest.json. Updated by the save system
 * after every successful save. Game-thread only.
 */
class GAMECORE_API FAtlasSaveSlotManifest
{
public:
	static bool AddSlot(const FString& SlotName, const FAtlasSaveGameMetadata& Metadata);
	static bool RemoveSlot(const FString& SlotName);
	static bool GetSlotMetadata(const FString& SlotName, FAtlasSaveGameMetadata& OutMetadata);
	static TArray<FAtlasSaveGameMetadata> GetAllSlots();

	static FString GetManifestPath();

private:
	static bool LoadManifest(TArray<FAtlasSaveGameMetadata>& OutSlots);
	static bool SaveManifest(const TArray<FAtlasSaveGameMetadata>& Slots);
};
