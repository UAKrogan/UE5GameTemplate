#include "Systems/Storage/AtlasFileStorage.h"

#include "HAL/FileManager.h"
#include "Logging/AtlasLogMacros.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace AtlasFileStorage
{
	constexpr int64 MaxSaveFileSizeBytes = 256ll * 1024ll * 1024ll;
}

bool FAtlasFileStorage::SaveToSlot(const FString& SlotName, const TArray<uint8>& Data)
{
	if (!IsValidSlotName(SlotName))
	{
		ATLAS_LOG_CORE(Error, "SaveToSlot failed: invalid slot name");
		return false;
	}

	if (Data.IsEmpty())
	{
		ATLAS_LOG_CORE(Error, "SaveToSlot failed: refusing to write empty data for slot=%s", *SlotName);
		return false;
	}

	if (!EnsureSaveDirectoryExists())
	{
		ATLAS_LOG_CORE(Error, "SaveToSlot failed: could not create save directory: %s", *GetSaveDirectory());
		return false;
	}

	const FString SlotFilePath = GetSlotFilePath(SlotName);
	const FString TempFilePath = GetTempSlotFilePath(SlotName);

	IFileManager& FileManager = IFileManager::Get();
	FileManager.Delete(*TempFilePath, false, true);

	if (!FFileHelper::SaveArrayToFile(Data, *TempFilePath))
	{
		FileManager.Delete(*TempFilePath, false, true);
		ATLAS_LOG_CORE(Error, "SaveToSlot failed: could not write temp file: %s", *TempFilePath);
		return false;
	}

	if (FileManager.FileSize(*TempFilePath) != Data.Num())
	{
		FileManager.Delete(*TempFilePath, false, true);
		ATLAS_LOG_CORE(Error, "SaveToSlot failed: temp file size mismatch for slot=%s", *SlotName);
		return false;
	}

	FileManager.Delete(*SlotFilePath, false, true);

	if (!FileManager.Move(*SlotFilePath, *TempFilePath, true, true, true, true))
	{
		FileManager.Delete(*TempFilePath, false, true);
		ATLAS_LOG_CORE(Error, "SaveToSlot failed: could not move temp file to slot file: slot=%s", *SlotName);
		return false;
	}

	ATLAS_LOG_CORE(Log, "Saved slot: slot=%s bytes=%d path=%s", *SlotName, Data.Num(), *SlotFilePath);
	return true;
}

bool FAtlasFileStorage::LoadFromSlot(const FString& SlotName, TArray<uint8>& OutData)
{
	OutData.Reset();

	if (!IsValidSlotName(SlotName))
	{
		ATLAS_LOG_CORE(Error, "LoadFromSlot failed: invalid slot name");
		return false;
	}

	const FString SlotFilePath = GetSlotFilePath(SlotName);
	IFileManager& FileManager = IFileManager::Get();

	if (!FileManager.FileExists(*SlotFilePath))
	{
		ATLAS_LOG_CORE(Warning, "LoadFromSlot failed: save file is missing: slot=%s path=%s", *SlotName, *SlotFilePath);
		return false;
	}

	const int64 FileSize = FileManager.FileSize(*SlotFilePath);
	if (FileSize <= 0 || FileSize > AtlasFileStorage::MaxSaveFileSizeBytes)
	{
		ATLAS_LOG_CORE(Error, "LoadFromSlot failed: save file appears corrupted: slot=%s size=%lld", *SlotName, FileSize);
		return false;
	}

	if (!FFileHelper::LoadFileToArray(OutData, *SlotFilePath))
	{
		OutData.Reset();
		ATLAS_LOG_CORE(Error, "LoadFromSlot failed: could not read save file: slot=%s path=%s", *SlotName, *SlotFilePath);
		return false;
	}

	if (OutData.Num() != FileSize)
	{
		OutData.Reset();
		ATLAS_LOG_CORE(Error, "LoadFromSlot failed: loaded data size mismatch: slot=%s expected=%lld actual=%d",
			*SlotName,
			FileSize,
			OutData.Num());
		return false;
	}

	ATLAS_LOG_CORE(Log, "Loaded slot: slot=%s bytes=%d path=%s", *SlotName, OutData.Num(), *SlotFilePath);
	return true;
}

bool FAtlasFileStorage::DoesSlotExist(const FString& SlotName)
{
	return IsValidSlotName(SlotName) && IFileManager::Get().FileExists(*GetSlotFilePath(SlotName));
}

bool FAtlasFileStorage::DeleteSlot(const FString& SlotName)
{
	if (!IsValidSlotName(SlotName))
	{
		return false;
	}

	const FString SlotFilePath = GetSlotFilePath(SlotName);
	const FString TempFilePath = GetTempSlotFilePath(SlotName);

	IFileManager& FileManager = IFileManager::Get();
	FileManager.Delete(*TempFilePath, false, true);

	if (!FileManager.FileExists(*SlotFilePath))
	{
		return true;
	}

	return FileManager.Delete(*SlotFilePath, false, true);
}

FString FAtlasFileStorage::GetSaveDirectory()
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"));
}

FString FAtlasFileStorage::GetSlotFilePath(const FString& SlotName)
{
	return FPaths::Combine(GetSaveDirectory(), SanitizeSlotName(SlotName) + SaveExtension);
}

FString FAtlasFileStorage::GetTempSlotFilePath(const FString& SlotName)
{
	return FPaths::Combine(GetSaveDirectory(), SanitizeSlotName(SlotName) + TempExtension);
}

bool FAtlasFileStorage::IsValidSlotName(const FString& SlotName)
{
	return !SanitizeSlotName(SlotName).IsEmpty();
}

FString FAtlasFileStorage::SanitizeSlotName(const FString& SlotName)
{
	FString SanitizedSlotName = SlotName;
	SanitizedSlotName.TrimStartAndEndInline();

	for (const TCHAR InvalidChar : FPaths::GetInvalidFileSystemChars())
	{
		SanitizedSlotName.ReplaceCharInline(InvalidChar, TEXT('_'));
	}

	SanitizedSlotName.ReplaceInline(TEXT("/"), TEXT("_"));
	SanitizedSlotName.ReplaceInline(TEXT("\\"), TEXT("_"));
	SanitizedSlotName.ReplaceInline(TEXT(".."), TEXT("_"));

	return SanitizedSlotName;
}

bool FAtlasFileStorage::EnsureSaveDirectoryExists()
{
	return IFileManager::Get().MakeDirectory(*GetSaveDirectory(), true);
}
