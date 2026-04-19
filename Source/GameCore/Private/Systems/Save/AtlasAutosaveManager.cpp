#include "Systems/Save/AtlasAutosaveManager.h"

#include "Logging/AtlasLogMacros.h"
#include "Systems/Storage/AtlasFileStorage.h"

#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

namespace AtlasAutosaveManager
{
	FCriticalSection AutosaveIndexCriticalSection;
	const TCHAR* IndexSlotName = TEXT("AutosaveIndex");
	const TCHAR* AutosaveSlotPrefix = TEXT("Autosave");
}

bool FAtlasAutosaveManager::ReserveNextAutosaveSlot(FString& OutSlotName, int32& OutSlotIndex)
{
	FScopeLock Lock(&AtlasAutosaveManager::AutosaveIndexCriticalSection);

	int32 CurrentIndex = INDEX_NONE;
	if (!LoadReservedIndex(CurrentIndex))
	{
		CurrentIndex = INDEX_NONE;
	}

	const int32 NextIndex = (CurrentIndex + 1 + AutosaveSlotCount) % AutosaveSlotCount;
	if (!StoreReservedIndex(NextIndex))
	{
		ATLAS_LOG_CORE(Error, "Failed to reserve autosave slot: nextIndex=%d", NextIndex);
		return false;
	}

	OutSlotIndex = NextIndex;
	OutSlotName = BuildAutosaveSlotName(NextIndex);

	ATLAS_LOG_CORE(Log, "Reserved autosave slot: slot=%s index=%d previousIndex=%d",
		*OutSlotName,
		OutSlotIndex,
		CurrentIndex);

	return true;
}

FString FAtlasAutosaveManager::BuildAutosaveSlotName(int32 SlotIndex)
{
	check(IsValidAutosaveIndex(SlotIndex));
	return FString::Printf(TEXT("%s_%d"), AtlasAutosaveManager::AutosaveSlotPrefix, SlotIndex);
}

FString FAtlasAutosaveManager::GetIndexSlotName()
{
	return AtlasAutosaveManager::IndexSlotName;
}

bool FAtlasAutosaveManager::LoadReservedIndex(int32& OutIndex)
{
	OutIndex = INDEX_NONE;

	TArray<uint8> IndexData;
	if (!FAtlasFileStorage::LoadFromSlot(GetIndexSlotName(), IndexData))
	{
		return false;
	}

	FMemoryReader MemoryReader(IndexData, true);
	int32 LoadedIndex = INDEX_NONE;
	MemoryReader << LoadedIndex;

	if (MemoryReader.IsError() || !IsValidAutosaveIndex(LoadedIndex))
	{
		ATLAS_LOG_CORE(Warning, "Autosave index file is missing or corrupted. Resetting autosave ring.");
		OutIndex = INDEX_NONE;
		return false;
	}

	OutIndex = LoadedIndex;
	return true;
}

bool FAtlasAutosaveManager::StoreReservedIndex(int32 Index)
{
	if (!IsValidAutosaveIndex(Index))
	{
		return false;
	}

	TArray<uint8> IndexData;
	FMemoryWriter MemoryWriter(IndexData, true);
	MemoryWriter << Index;

	if (MemoryWriter.IsError() || IndexData.IsEmpty())
	{
		return false;
	}

	return FAtlasFileStorage::SaveToSlot(GetIndexSlotName(), IndexData);
}

bool FAtlasAutosaveManager::IsValidAutosaveIndex(int32 Index)
{
	return Index >= 0 && Index < AutosaveSlotCount;
}
