#include "Systems/Load/AtlasLoadSystem.h"

#include "SaveGame/AtlasSaveGame.h"
#include "Logging/AtlasLogMacros.h"

#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"

namespace AtlasLoadSystem
{
	constexpr int32 DefaultUserIndex = 0;
	const TCHAR* DefaultSlotSuffix = TEXT("MainProfile");
}

/*
 * Called during system initialization.
 */
void FAtlasLoadSystem::Initialize(UAtlasGameInstanceSubsystem* Subsystem)
{
	OwningSubsystem = Subsystem;
	LoadedSnapshot.Reset();

	ATLAS_LOG_CORE(Log, "LoadSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasLoadSystem::Shutdown()
{
	LoadedSnapshot.Reset();
	OwningSubsystem = nullptr;

	ATLAS_LOG_CORE(Log, "LoadSystem shutdown");
}

/*
 * Entry point for loading game state.
 */
void FAtlasLoadSystem::LoadGame()
{
	LoadGameFromSlot(GetDefaultSlotName(), AtlasLoadSystem::DefaultUserIndex);
}

bool FAtlasLoadSystem::LoadGameFromSlot(const FString& SlotName, int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ATLAS_LOG_CORE(Error, "LoadGameFromSlot failed: slot name is empty");
		return false;
	}

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		ATLAS_LOG_CORE(Warning, "LoadGameFromSlot failed: slot does not exist: slot=%s userIndex=%d", *SlotName, UserIndex);
		return false;
	}

	USaveGame* RawSaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	UAtlasSaveGame* SaveGameObject = Cast<UAtlasSaveGame>(RawSaveGame);
	if (!SaveGameObject)
	{
		ATLAS_LOG_CORE(Error, "LoadGameFromSlot failed: slot=%s userIndex=%d is not a UAtlasSaveGame", *SlotName, UserIndex);
		return false;
	}

	LoadedSnapshot = SaveGameObject->GetSnapshot();

	ATLAS_LOG_CORE(Log, "Loaded snapshot: slot=%s userIndex=%d saveId=%s entries=%d",
		*LoadedSnapshot.Metadata.SlotName,
		LoadedSnapshot.Metadata.UserIndex,
		*LoadedSnapshot.Metadata.SaveId,
		LoadedSnapshot.IntValues.Num() +
			LoadedSnapshot.FloatValues.Num() +
			LoadedSnapshot.StringValues.Num() +
			LoadedSnapshot.VectorValues.Num() +
			LoadedSnapshot.RotatorValues.Num() +
			LoadedSnapshot.TransformValues.Num());

	return true;
}

FString FAtlasLoadSystem::GetDefaultSlotName() const
{
	const FString ProjectName = FString(FApp::GetProjectName());
	if (ProjectName.IsEmpty())
	{
		return TEXT("Atlas_MainProfile");
	}

	return FString::Printf(TEXT("%s_%s"), *ProjectName, AtlasLoadSystem::DefaultSlotSuffix);
}

bool FAtlasLoadSystem::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
	if (SlotName.IsEmpty())
	{
		return false;
	}

	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

void FAtlasLoadSystem::ClearLoadedSnapshot()
{
	LoadedSnapshot.Reset();
}

bool FAtlasLoadSystem::GetLoadedMetadata(FAtlasSaveGameMetadata& OutMetadata) const
{
	OutMetadata = LoadedSnapshot.Metadata;
	return LoadedSnapshot.Metadata.IsValid();
}

bool FAtlasLoadSystem::GetLoadedSnapshot(FAtlasSaveGameSnapshot& OutSnapshot) const
{
	OutSnapshot = LoadedSnapshot;
	return LoadedSnapshot.Metadata.IsValid() || !LoadedSnapshot.IsEmpty();
}

bool FAtlasLoadSystem::TryGetIntValue(FName Key, int32& OutValue) const
{
	if (const int32* Value = LoadedSnapshot.IntValues.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

bool FAtlasLoadSystem::TryGetFloatValue(FName Key, float& OutValue) const
{
	if (const float* Value = LoadedSnapshot.FloatValues.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

bool FAtlasLoadSystem::TryGetStringValue(FName Key, FString& OutValue) const
{
	if (const FString* Value = LoadedSnapshot.StringValues.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

bool FAtlasLoadSystem::TryGetVectorValue(FName Key, FVector& OutValue) const
{
	if (const FVector* Value = LoadedSnapshot.VectorValues.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

bool FAtlasLoadSystem::TryGetRotatorValue(FName Key, FRotator& OutValue) const
{
	if (const FRotator* Value = LoadedSnapshot.RotatorValues.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}

bool FAtlasLoadSystem::TryGetTransformValue(FName Key, FTransform& OutValue) const
{
	if (const FTransform* Value = LoadedSnapshot.TransformValues.Find(Key))
	{
		OutValue = *Value;
		return true;
	}

	return false;
}
