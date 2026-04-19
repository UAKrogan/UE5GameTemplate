#include "Systems/Load/AtlasLoadSystem.h"

#include "Logging/AtlasLogMacros.h"
#include "Subsystems/AtlasGameInstanceSubsystem.h"
#include "Systems/Savable/AtlasSavable.h"
#include "Systems/Savable/AtlasSavableComponent.h"
#include "Systems/Serialization/AtlasBinaryReader.h"
#include "Systems/Storage/AtlasFileStorage.h"

#include "Async/Async.h"
#include "Components/ActorComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Misc/App.h"
#include "Serialization/MemoryReader.h"

namespace AtlasLoadSystem
{
	constexpr int32 DefaultUserIndex = 0;
	const TCHAR* DefaultSlotSuffix = TEXT("MainProfile");
	const FName ActorChunkName = TEXT("Actor");
	const TCHAR* ComponentChunkPrefix = TEXT("Component");
}

/*
 * Called during system initialization.
 */
void FAtlasLoadSystem::Initialize(UAtlasGameInstanceSubsystem* Subsystem)
{
	OwningSubsystem = Subsystem;
	LoadedSnapshot.Reset();
	LoadedWorldSnapshot.Reset();
	bLoadInProgress = false;
	bShuttingDown = false;

	ATLAS_LOG_CORE(Log, "LoadSystem initialized");
}

/*
 * Called during system shutdown.
 */
void FAtlasLoadSystem::Shutdown()
{
	bShuttingDown = true;
	LoadedSnapshot.Reset();
	LoadedWorldSnapshot.Reset();
	OwningSubsystem = nullptr;

	ATLAS_LOG_CORE(Log, "LoadSystem shutdown");
}

/*
 * Entry point for loading game state.
 */
void FAtlasLoadSystem::LoadGame()
{
	RequestLoad(GetDefaultSlotName());
}

bool FAtlasLoadSystem::RequestLoad(const FString& SlotName)
{
	check(IsInGameThread());

	if (bShuttingDown)
	{
		ATLAS_LOG_CORE(Warning, "Load request rejected during shutdown: slot=%s", *SlotName);
		return false;
	}

	if (bLoadInProgress)
	{
		ATLAS_LOG_CORE(Warning, "Load request rejected because another load is in progress: slot=%s", *SlotName);
		return false;
	}

	if (SlotName.IsEmpty())
	{
		ATLAS_LOG_CORE(Error, "Load request rejected: slot name is empty");
		return false;
	}

	bLoadInProgress = true;
	TWeakPtr<FAtlasLoadSystem> WeakLoadSystem = AsShared();

	ATLAS_LOG_CORE(Log, "Load request started: slot=%s", *SlotName);

	Async(EAsyncExecution::ThreadPool, [WeakLoadSystem, SlotName]()
	{
		TArray<uint8> BinaryData;
		FAtlasWorldSnapshot WorldSnapshot;

		const bool bLoaded = FAtlasFileStorage::LoadFromSlot(SlotName, BinaryData);
		const bool bDeserialized = bLoaded && FAtlasBinaryReader::Deserialize(BinaryData, WorldSnapshot);
		const bool bSuccess = bLoaded && bDeserialized;

		AsyncTask(ENamedThreads::GameThread, [WeakLoadSystem, SlotName, bSuccess, WorldSnapshot = MoveTemp(WorldSnapshot)]() mutable
		{
			if (TSharedPtr<FAtlasLoadSystem> LoadSystem = WeakLoadSystem.Pin())
			{
				LoadSystem->HandleSnapshotLoaded(SlotName, bSuccess, MoveTemp(WorldSnapshot));
			}
		});
	});

	return true;
}

bool FAtlasLoadSystem::LoadGameFromSlot(const FString& SlotName, int32 UserIndex)
{
	if (UserIndex != AtlasLoadSystem::DefaultUserIndex)
	{
		ATLAS_LOG_CORE(Warning, "LoadGameFromSlot ignores UserIndex for Atlas binary storage: slot=%s userIndex=%d", *SlotName, UserIndex);
	}

	return RequestLoad(SlotName);
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
	if (UserIndex != AtlasLoadSystem::DefaultUserIndex)
	{
		ATLAS_LOG_CORE(Warning, "DoesSaveExist ignores UserIndex for Atlas binary storage: slot=%s userIndex=%d", *SlotName, UserIndex);
	}

	return FAtlasFileStorage::DoesSlotExist(SlotName);
}

void FAtlasLoadSystem::ClearLoadedSnapshot()
{
	LoadedSnapshot.Reset();
	LoadedWorldSnapshot.Reset();
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

UWorld* FAtlasLoadSystem::ResolveWorld() const
{
	if (!OwningSubsystem)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = OwningSubsystem->GetGameInstance();
	return GameInstance ? GameInstance->GetWorld() : nullptr;
}

AActor* FAtlasLoadSystem::FindActorById(UWorld* World, const FGuid& ActorId) const
{
	if (!World || !ActorId.IsValid())
	{
		return nullptr;
	}

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		const UAtlasSavableComponent* SavableComponent = UAtlasSavableComponent::FindOnActor(Actor);
		if (SavableComponent && SavableComponent->GetActorId() == ActorId)
		{
			return Actor;
		}
	}

	return nullptr;
}

AActor* FAtlasLoadSystem::SpawnActorFromSnapshot(UWorld* World, const FAtlasActorSnapshot& ActorSnapshot) const
{
	if (!World || ActorSnapshot.ClassPath.IsEmpty())
	{
		return nullptr;
	}

	UClass* ActorClass = LoadObject<UClass>(nullptr, *ActorSnapshot.ClassPath);
	if (!ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
	{
		ATLAS_LOG_CORE(Error, "Failed to load actor class for snapshot: actorId=%s classPath=%s",
			*ActorSnapshot.ActorId.ToString(),
			*ActorSnapshot.ClassPath);
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, ActorSnapshot.Transform, SpawnParameters);
	if (!SpawnedActor)
	{
		ATLAS_LOG_CORE(Error, "Failed to spawn actor from snapshot: actorId=%s classPath=%s",
			*ActorSnapshot.ActorId.ToString(),
			*ActorSnapshot.ClassPath);
		return nullptr;
	}

	EnsureSavableComponent(SpawnedActor, ActorSnapshot.ActorId);

	ATLAS_LOG_CORE(Log, "Spawned missing actor from snapshot: actorId=%s name=%s classPath=%s",
		*ActorSnapshot.ActorId.ToString(),
		*SpawnedActor->GetName(),
		*ActorSnapshot.ClassPath);

	return SpawnedActor;
}

UAtlasSavableComponent* FAtlasLoadSystem::EnsureSavableComponent(AActor* Actor, const FGuid& ActorId) const
{
	if (!Actor || !ActorId.IsValid())
	{
		return nullptr;
	}

	UAtlasSavableComponent* SavableComponent = UAtlasSavableComponent::FindOnActor(Actor);
	if (!SavableComponent)
	{
		SavableComponent = NewObject<UAtlasSavableComponent>(Actor, UAtlasSavableComponent::StaticClass(), TEXT("AtlasSavableComponent"));
		if (SavableComponent)
		{
			Actor->AddInstanceComponent(SavableComponent);
			SavableComponent->RegisterComponent();
		}
	}

	if (SavableComponent)
	{
		SavableComponent->SetActorId(ActorId);
	}

	return SavableComponent;
}

void FAtlasLoadSystem::HandleSnapshotLoaded(const FString& SlotName, bool bSuccess, FAtlasWorldSnapshot&& WorldSnapshot)
{
	check(IsInGameThread());

	bLoadInProgress = false;

	if (bShuttingDown)
	{
		ATLAS_LOG_CORE(Warning, "Discarding loaded snapshot during shutdown: slot=%s", *SlotName);
		return;
	}

	if (!bSuccess)
	{
		ATLAS_LOG_CORE(Error, "Load failed before restore: slot=%s", *SlotName);
		return;
	}

	LoadedWorldSnapshot = MoveTemp(WorldSnapshot);
	ATLAS_LOG_CORE(Log, "Load deserialized snapshot: slot=%s actors=%d", *SlotName, LoadedWorldSnapshot.Actors.Num());

	ApplyWorldSnapshot(SlotName, LoadedWorldSnapshot);
}

void FAtlasLoadSystem::ApplyWorldSnapshot(const FString& SlotName, const FAtlasWorldSnapshot& WorldSnapshot)
{
	check(IsInGameThread());

	UWorld* World = ResolveWorld();
	if (!World)
	{
		ATLAS_LOG_CORE(Error, "Load restore failed: world is null slot=%s", *SlotName);
		return;
	}

	TMap<FGuid, TWeakObjectPtr<AActor>> ActorsById;

	ATLAS_LOG_CORE(Log, "Load restore phase 1 started: slot=%s", *SlotName);
	ResolveActorsForSnapshot(World, WorldSnapshot, ActorsById);

	ATLAS_LOG_CORE(Log, "Load restore phase 2 started: slot=%s", *SlotName);
	RestoreSnapshotState(WorldSnapshot, ActorsById);

	ATLAS_LOG_CORE(Log, "Load restore completed: slot=%s actors=%d", *SlotName, ActorsById.Num());
}

void FAtlasLoadSystem::ResolveActorsForSnapshot(UWorld* World, const FAtlasWorldSnapshot& WorldSnapshot, TMap<FGuid, TWeakObjectPtr<AActor>>& OutActorsById)
{
	for (const FAtlasActorSnapshot& ActorSnapshot : WorldSnapshot.Actors)
	{
		if (!ActorSnapshot.ActorId.IsValid())
		{
			ATLAS_LOG_CORE(Warning, "Skipping actor snapshot with invalid ActorId: classPath=%s", *ActorSnapshot.ClassPath);
			continue;
		}

		AActor* Actor = FindActorById(World, ActorSnapshot.ActorId);
		if (!Actor)
		{
			Actor = SpawnActorFromSnapshot(World, ActorSnapshot);
		}

		if (!Actor)
		{
			ATLAS_LOG_CORE(Error, "Could not resolve actor for snapshot: actorId=%s classPath=%s",
				*ActorSnapshot.ActorId.ToString(),
				*ActorSnapshot.ClassPath);
			continue;
		}

		EnsureSavableComponent(Actor, ActorSnapshot.ActorId);
		Actor->SetActorTransform(ActorSnapshot.Transform);
		OutActorsById.Add(ActorSnapshot.ActorId, Actor);
	}
}

void FAtlasLoadSystem::RestoreSnapshotState(const FAtlasWorldSnapshot& WorldSnapshot, const TMap<FGuid, TWeakObjectPtr<AActor>>& ActorsById)
{
	for (const FAtlasActorSnapshot& ActorSnapshot : WorldSnapshot.Actors)
	{
		const TWeakObjectPtr<AActor>* ActorPtr = ActorsById.Find(ActorSnapshot.ActorId);
		AActor* Actor = ActorPtr ? ActorPtr->Get() : nullptr;
		if (!Actor)
		{
			ATLAS_LOG_CORE(Warning, "Skipping restore for unresolved actor: actorId=%s", *ActorSnapshot.ActorId.ToString());
			continue;
		}

		RestoreActorState(Actor, ActorSnapshot);
		RestoreComponentState(Actor, ActorSnapshot);
	}
}

void FAtlasLoadSystem::RestoreActorState(AActor* Actor, const FAtlasActorSnapshot& ActorSnapshot)
{
	if (!Actor || !Actor->GetClass()->ImplementsInterface(UAtlasSavable::StaticClass()))
	{
		return;
	}

	const FAtlasDataChunk* ActorChunk = ActorSnapshot.FindChunk(BuildActorChunkName());
	if (!ActorChunk)
	{
		ATLAS_LOG_CORE(Verbose, "Actor has no actor chunk to restore: actor=%s", *GetNameSafe(Actor));
		return;
	}

	RestoreSavableObject(Actor, *ActorChunk);
}

void FAtlasLoadSystem::RestoreComponentState(AActor* Actor, const FAtlasActorSnapshot& ActorSnapshot)
{
	if (!Actor)
	{
		return;
	}

	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);

	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ++ComponentIndex)
	{
		UActorComponent* Component = Components[ComponentIndex];
		if (!IsValid(Component) || !Component->GetClass()->ImplementsInterface(UAtlasSavable::StaticClass()))
		{
			continue;
		}

		const FName ChunkName = BuildComponentChunkName(Component, ComponentIndex);
		const FAtlasDataChunk* ComponentChunk = ActorSnapshot.FindChunk(ChunkName);
		if (!ComponentChunk)
		{
			ATLAS_LOG_CORE(Warning, "Missing component chunk during restore: actor=%s component=%s chunk=%s",
				*GetNameSafe(Actor),
				*GetNameSafe(Component),
				*ChunkName.ToString());
			continue;
		}

		RestoreSavableObject(Component, *ComponentChunk);
	}
}

bool FAtlasLoadSystem::RestoreSavableObject(UObject* Object, const FAtlasDataChunk& DataChunk)
{
	if (!IsValid(Object) || DataChunk.Name.IsNone() || DataChunk.Data.IsEmpty())
	{
		return false;
	}

	IAtlasSavable* SavableObject = Cast<IAtlasSavable>(Object);
	if (!SavableObject)
	{
		return false;
	}

	FMemoryReader MemoryReader(DataChunk.Data, true);
	FAtlasLoadContext LoadContext(MemoryReader);

	if (!LoadContext.BeginChunk(DataChunk.Name))
	{
		ATLAS_LOG_CORE(Error, "Failed to begin load chunk: object=%s chunk=%s", *GetNameSafe(Object), *DataChunk.Name.ToString());
		return false;
	}

	SavableObject->RestoreState(LoadContext);
	LoadContext.EndChunk();

	ATLAS_LOG_CORE(Verbose, "Restored savable object: object=%s chunk=%s bytes=%d",
		*GetNameSafe(Object),
		*DataChunk.Name.ToString(),
		DataChunk.Data.Num());

	return true;
}

FName FAtlasLoadSystem::BuildActorChunkName()
{
	return AtlasLoadSystem::ActorChunkName;
}

FName FAtlasLoadSystem::BuildComponentChunkName(const UActorComponent* Component, int32 ComponentIndex)
{
	check(Component != nullptr);

	return FName(*FString::Printf(
		TEXT("%s.%d.%s"),
		AtlasLoadSystem::ComponentChunkPrefix,
		ComponentIndex,
		*Component->GetName()));
}
