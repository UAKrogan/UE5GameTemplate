#include "Systems/Save/AtlasSaveSystem.h"

#include "Logging/AtlasLogMacros.h"
#include "Subsystems/AtlasGameInstanceSubsystem.h"
#include "Systems/Save/AtlasAutosaveManager.h"
#include "Systems/Save/AtlasSaveCollector.h"
#include "Systems/Serialization/AtlasBinaryWriter.h"
#include "Systems/Storage/AtlasFileStorage.h"

#include "Async/Async.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
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
	PendingSaveRequests.Reset();
	bSaveInProgress = false;
	bShuttingDown = false;

	ATLAS_LOG_CORE(Log, "SaveSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasSaveSystem::Shutdown()
{
	bShuttingDown = true;
	PendingSaveRequests.Reset();
	PendingSnapshot.Reset();
	OwningSubsystem = nullptr;

	ATLAS_LOG_CORE(Log, "SaveSystem shutdown");
}

/*
 * Entry point for saving game state.
 */
void FAtlasSaveSystem::SaveGame()
{
	RequestSave(GetDefaultSlotName());
}

bool FAtlasSaveSystem::RequestSave(const FString& SlotName)
{
	return EnqueueSaveRequest(SlotName, false);
}

bool FAtlasSaveSystem::RequestAutosave()
{
	FString AutosaveSlotName;
	int32 AutosaveIndex = INDEX_NONE;
	if (!FAtlasAutosaveManager::ReserveNextAutosaveSlot(AutosaveSlotName, AutosaveIndex))
	{
		ATLAS_LOG_CORE(Error, "Autosave request rejected: failed to reserve autosave slot");
		return false;
	}

	return EnqueueSaveRequest(AutosaveSlotName, true, AutosaveIndex);
}

bool FAtlasSaveSystem::SaveGameToSlot(const FString& SlotName, int32 UserIndex)
{
	if (UserIndex != AtlasSaveSystem::DefaultUserIndex)
	{
		ATLAS_LOG_CORE(Warning, "SaveGameToSlot ignores UserIndex for Atlas binary storage: slot=%s userIndex=%d", *SlotName, UserIndex);
	}

	return RequestSave(SlotName);
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
	if (UserIndex != AtlasSaveSystem::DefaultUserIndex)
	{
		ATLAS_LOG_CORE(Warning, "DoesSaveExist ignores UserIndex for Atlas binary storage: slot=%s userIndex=%d", *SlotName, UserIndex);
	}

	return FAtlasFileStorage::DoesSlotExist(SlotName);
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
	const UWorld* World = ResolveWorld();
	if (!World)
	{
		return FString();
	}

	FString MapName = World->GetMapName();
	MapName.RemoveFromStart(World->StreamingLevelsPrefix);
	return MapName;
}

UWorld* FAtlasSaveSystem::ResolveWorld() const
{
	if (!OwningSubsystem)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = OwningSubsystem->GetGameInstance();
	return GameInstance ? GameInstance->GetWorld() : nullptr;
}

bool FAtlasSaveSystem::EnqueueSaveRequest(const FString& SlotName, bool bAutosave, int32 AutosaveIndex)
{
	check(IsInGameThread());

	if (bShuttingDown)
	{
		ATLAS_LOG_CORE(Warning, "Save request rejected during shutdown: slot=%s", *SlotName);
		return false;
	}

	if (SlotName.IsEmpty())
	{
		ATLAS_LOG_CORE(Error, "Save request rejected: slot name is empty");
		return false;
	}

	FAtlasQueuedSaveRequest Request;
	Request.SlotName = SlotName;
	Request.bAutosave = bAutosave;
	Request.AutosaveIndex = AutosaveIndex;
	PendingSaveRequests.Add(MoveTemp(Request));

	ATLAS_LOG_CORE(Log, "Save request queued: slot=%s autosave=%s autosaveIndex=%d queueSize=%d",
		*SlotName,
		bAutosave ? TEXT("true") : TEXT("false"),
		AutosaveIndex,
		PendingSaveRequests.Num());

	ProcessNextSaveRequest();
	return true;
}

void FAtlasSaveSystem::ProcessNextSaveRequest()
{
	check(IsInGameThread());

	if (bShuttingDown || bSaveInProgress || PendingSaveRequests.IsEmpty())
	{
		return;
	}

	FAtlasQueuedSaveRequest Request = MoveTemp(PendingSaveRequests[0]);
	PendingSaveRequests.RemoveAt(0);
	StartSaveRequest(Request);
}

void FAtlasSaveSystem::StartSaveRequest(const FAtlasQueuedSaveRequest& Request)
{
	check(IsInGameThread());

	UWorld* World = ResolveWorld();
	if (!World)
	{
		ATLAS_LOG_CORE(Error, "Save request failed: world is null slot=%s", *Request.SlotName);
		ProcessNextSaveRequest();
		return;
	}

	bSaveInProgress = true;

	ATLAS_LOG_CORE(Log, "Save collection started: slot=%s autosave=%s",
		*Request.SlotName,
		Request.bAutosave ? TEXT("true") : TEXT("false"));

	FAtlasWorldSnapshot WorldSnapshot = FAtlasSaveCollector::CollectWorld(World);
	const int32 ActorCount = WorldSnapshot.Actors.Num();
	const FString SlotName = Request.SlotName;
	const bool bAutosave = Request.bAutosave;
	const int32 AutosaveIndex = Request.AutosaveIndex;
	TWeakPtr<FAtlasSaveSystem> WeakSaveSystem = AsShared();

	ATLAS_LOG_CORE(Log, "Save collection finished: slot=%s autosaveIndex=%d actors=%d", *SlotName, AutosaveIndex, ActorCount);

	Async(EAsyncExecution::ThreadPool, [WeakSaveSystem, SlotName, bAutosave, ActorCount, WorldSnapshot = MoveTemp(WorldSnapshot)]() mutable
	{
		TArray<uint8> BinaryData;
		const bool bSerialized = FAtlasBinaryWriter::Serialize(WorldSnapshot, BinaryData);
		const bool bSaved = bSerialized && FAtlasFileStorage::SaveToSlot(SlotName, BinaryData);
		const int32 ByteCount = BinaryData.Num();

		AsyncTask(ENamedThreads::GameThread, [WeakSaveSystem, SlotName, bAutosave, bSaved, ActorCount, ByteCount]()
		{
			if (TSharedPtr<FAtlasSaveSystem> SaveSystem = WeakSaveSystem.Pin())
			{
				SaveSystem->HandleSaveCompleted(SlotName, bAutosave, bSaved, ActorCount, ByteCount);
			}
		});
	});
}

void FAtlasSaveSystem::HandleSaveCompleted(const FString& SlotName, bool bAutosave, bool bSuccess, int32 ActorCount, int32 ByteCount)
{
	check(IsInGameThread());

	bSaveInProgress = false;

	if (bSuccess)
	{
		ATLAS_LOG_CORE(Log, "Save completed: slot=%s autosave=%s actors=%d bytes=%d",
			*SlotName,
			bAutosave ? TEXT("true") : TEXT("false"),
			ActorCount,
			ByteCount);
	}
	else
	{
		ATLAS_LOG_CORE(Error, "Save failed: slot=%s autosave=%s actors=%d bytes=%d",
			*SlotName,
			bAutosave ? TEXT("true") : TEXT("false"),
			ActorCount,
			ByteCount);
	}

	ProcessNextSaveRequest();
}
