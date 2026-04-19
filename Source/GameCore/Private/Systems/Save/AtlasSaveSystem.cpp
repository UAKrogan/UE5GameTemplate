#include "Systems/Save/AtlasSaveSystem.h"

#include "SaveGame/AtlasSaveGame.h"
#include "Logging/AtlasLogMacros.h"
#include "Subsystems/AtlasGameInstanceSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/DateTime.h"
#include "Misc/Guid.h"

namespace AtlasSaveSystem
{
	constexpr int32 CurrentDataVersion = 1;
	constexpr int32 DefaultUserIndex = 0;
	const TCHAR* DefaultSlotSuffix = TEXT("MainProfile");
}

/*
 * Called during system initialization.
 */
void FAtlasSaveSystem::Initialize(UAtlasGameInstanceSubsystem* Subsystem)
{
	OwningSubsystem = Subsystem;
	PendingSnapshot.Reset();

	ATLAS_LOG_CORE(Log, "SaveSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasSaveSystem::Shutdown()
{
	PendingSnapshot.Reset();
	OwningSubsystem = nullptr;

	ATLAS_LOG_CORE(Log, "SaveSystem shutdown");
}

/*
 * Entry point for saving game state.
 */
void FAtlasSaveSystem::SaveGame()
{
	SaveGameToSlot(GetDefaultSlotName(), AtlasSaveSystem::DefaultUserIndex);
}

bool FAtlasSaveSystem::SaveGameToSlot(const FString& SlotName, int32 UserIndex)
{
	if (SlotName.IsEmpty())
	{
		ATLAS_LOG_CORE(Error, "SaveGameToSlot failed: slot name is empty");
		return false;
	}

	UAtlasSaveGame* SaveGameObject = Cast<UAtlasSaveGame>(UGameplayStatics::CreateSaveGameObject(UAtlasSaveGame::StaticClass()));
	if (!SaveGameObject)
	{
		ATLAS_LOG_CORE(Error, "SaveGameToSlot failed: could not create UAtlasSaveGame object");
		return false;
	}

	const FAtlasSaveGameSnapshot SnapshotToSave = BuildSnapshotForSave(SlotName, UserIndex);
	SaveGameObject->SetSnapshot(SnapshotToSave);

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex);
	if (!bSaved)
	{
		ATLAS_LOG_CORE(Error, "SaveGameToSlot failed: slot=%s userIndex=%d", *SlotName, UserIndex);
		return false;
	}

	PendingSnapshot = SnapshotToSave;

	ATLAS_LOG_CORE(Log, "Saved snapshot: slot=%s userIndex=%d saveId=%s entries=%d",
		*SnapshotToSave.Metadata.SlotName,
		SnapshotToSave.Metadata.UserIndex,
		*SnapshotToSave.Metadata.SaveId,
		SnapshotToSave.IntValues.Num() +
			SnapshotToSave.FloatValues.Num() +
			SnapshotToSave.StringValues.Num() +
			SnapshotToSave.VectorValues.Num() +
			SnapshotToSave.RotatorValues.Num() +
			SnapshotToSave.TransformValues.Num());

	return true;
}

FString FAtlasSaveSystem::GetDefaultSlotName() const
{
	const FString ProjectName = FString(FApp::GetProjectName());
	if (ProjectName.IsEmpty())
	{
		return TEXT("Atlas_MainProfile");
	}

	return FString::Printf(TEXT("%s_%s"), *ProjectName, AtlasSaveSystem::DefaultSlotSuffix);
}

bool FAtlasSaveSystem::DoesSaveExist(const FString& SlotName, int32 UserIndex) const
{
	if (SlotName.IsEmpty())
	{
		return false;
	}

	return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

void FAtlasSaveSystem::ClearPendingSnapshot()
{
	PendingSnapshot.Reset();
}

void FAtlasSaveSystem::SetIntValue(FName Key, int32 Value)
{
	PendingSnapshot.IntValues.Add(Key, Value);
}

void FAtlasSaveSystem::SetFloatValue(FName Key, float Value)
{
	PendingSnapshot.FloatValues.Add(Key, Value);
}

void FAtlasSaveSystem::SetStringValue(FName Key, const FString& Value)
{
	PendingSnapshot.StringValues.Add(Key, Value);
}

void FAtlasSaveSystem::SetVectorValue(FName Key, const FVector& Value)
{
	PendingSnapshot.VectorValues.Add(Key, Value);
}

void FAtlasSaveSystem::SetRotatorValue(FName Key, const FRotator& Value)
{
	PendingSnapshot.RotatorValues.Add(Key, Value);
}

void FAtlasSaveSystem::SetTransformValue(FName Key, const FTransform& Value)
{
	PendingSnapshot.TransformValues.Add(Key, Value);
}

bool FAtlasSaveSystem::RemoveValue(FName Key)
{
	bool bRemoved = false;

	bRemoved |= PendingSnapshot.IntValues.Remove(Key) > 0;
	bRemoved |= PendingSnapshot.FloatValues.Remove(Key) > 0;
	bRemoved |= PendingSnapshot.StringValues.Remove(Key) > 0;
	bRemoved |= PendingSnapshot.VectorValues.Remove(Key) > 0;
	bRemoved |= PendingSnapshot.RotatorValues.Remove(Key) > 0;
	bRemoved |= PendingSnapshot.TransformValues.Remove(Key) > 0;

	return bRemoved;
}

bool FAtlasSaveSystem::GetPendingSnapshot(FAtlasSaveGameSnapshot& OutSnapshot) const
{
	OutSnapshot = PendingSnapshot;
	return PendingSnapshot.Metadata.IsValid() || !PendingSnapshot.IsEmpty();
}

FAtlasSaveGameSnapshot FAtlasSaveSystem::BuildSnapshotForSave(const FString& SlotName, int32 UserIndex) const
{
	FAtlasSaveGameSnapshot Snapshot = PendingSnapshot;
	Snapshot.Metadata.DataVersion = AtlasSaveSystem::CurrentDataVersion;
	Snapshot.Metadata.SlotName = SlotName;
	Snapshot.Metadata.UserIndex = UserIndex;
	Snapshot.Metadata.SaveId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	Snapshot.Metadata.SavedAtUtc = FDateTime::UtcNow().ToIso8601();

	const FString BuildVersion = FApp::GetBuildVersion();
	Snapshot.Metadata.BuildVersion = BuildVersion.IsEmpty() ? TEXT("Development") : BuildVersion;
	Snapshot.Metadata.MapName = ResolveCurrentMapName();

	return Snapshot;
}

FString FAtlasSaveSystem::ResolveCurrentMapName() const
{
	if (!OwningSubsystem)
	{
		return FString();
	}

	const UGameInstance* GameInstance = OwningSubsystem->GetGameInstance();
	if (!GameInstance)
	{
		return FString();
	}

	const UWorld* World = GameInstance->GetWorld();
	if (!World)
	{
		return FString();
	}

	FString MapName = World->GetMapName();
	MapName.RemoveFromStart(World->StreamingLevelsPrefix);
	return MapName;
}
